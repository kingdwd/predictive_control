
//This file containts read parameter from server, callback, call class objects, control all class, objects of all class

#include <lmpcc/mpcc_controller.h>
#include <nav_msgs/Odometry.h>
#include <geometry_msgs/Pose.h>

ACADOvariables acadoVariables;
ACADOworkspace acadoWorkspace;

MPCC::~MPCC()
{
}

void MPCC::spinNode()
{
    ROS_INFO(" Predictive control node is running, now it's 'Spinning Node'");
    ros::spin();
}

// initialize all helper class of predictive control and subscibe joint state and publish controlled joint velocity
bool MPCC::initialize()
{
    // make sure node is still running
    if (ros::ok())
    {
        // initialize helper classes, make sure pd_config should be initialized first as mother of other class
        controller_config_.reset(new predictive_configuration());
        bool controller_config_success = controller_config_->initialize();

        bool kinematic_success = true;

        if (controller_config_success == false)
        {
            ROS_ERROR("MPCC: FAILED TO INITILIZED!!");
            std::cout << "States: \n"
                      << " pd_config: " << std::boolalpha << controller_config_success << "\n"
                      << " pd config init success: " << std::boolalpha << controller_config_->initialize_success_
                      << std::endl;
            return false;
        }

        // initialize data member of class
        clock_frequency_ = controller_config_->clock_frequency_;

        //DEBUG
        activate_debug_output_ = controller_config_->activate_debug_output_;
        plotting_result_ = controller_config_->plotting_result_;

        // DEBUG
        if (controller_config_->activate_controller_node_output_)
        {
            ROS_WARN("===== DEBUG INFO ACTIVATED =====");
        }

        // resize position and velocity velocity vectors
        current_state_ = Eigen::Vector4d(0,0,0,0);
        last_state_ = Eigen::Vector4d(0,0,0,0);

        waypoints_size_ = 0; // min 2 waypoints

        robot_state_sub_ = nh.subscribe(controller_config_->robot_state_topic_, 1, &MPCC::StateCallBack, this);

        obstacle_feed_sub_ = nh.subscribe(controller_config_->sub_ellipse_topic_, 1, &MPCC::ObstacleCallBack, this);

        obstacles_state_sub_ = nh.subscribe(controller_config_->obs_state_topic_, 1, &MPCC::ObstacleStateCallback, this);

        waypoints_sub_ = nh.subscribe(controller_config_->waypoint_topic_,1, &MPCC::getWayPointsCallBack, this);

        //Publishers
        traj_pub_ = nh.advertise<visualization_msgs::MarkerArray>("pd_trajectory",1);
        pred_cmd_pub_ = nh.advertise<nav_msgs::Path>("predicted_cmd",1);
        cost_pub_ = nh.advertise<std_msgs::Float64>("cost",1);
        brake_pub_ = nh.advertise<std_msgs::Float64>("break",1);
        contour_error_pub_ = nh.advertise<std_msgs::Float64MultiArray>("contour_error",1);
        controlled_velocity_pub_ = nh.advertise<lmpcc::Control>(controller_config_->output_cmd,1);
        joint_state_pub_ = nh.advertise<sensor_msgs::JointState>("/joint_states",1);
        robot_collision_space_pub_ = nh.advertise<visualization_msgs::MarkerArray>("/robot_collision_space", 100);
        pred_traj_pub_ = nh.advertise<nav_msgs::Path>("predicted_trajectory",1);
        spline_traj_pub_ = nh.advertise<nav_msgs::Path>("spline_traj",1);
        feedback_pub_ = nh.advertise<lmpcc::control_feedback>("controller_feedback",1);
        //Road publisher
        marker_pub_ = nh.advertise<visualization_msgs::MarkerArray>("road", 10);
        ros::Duration(1).sleep();

        timer_ = nh.createTimer(ros::Duration(1/clock_frequency_), &MPCC::runNode, this);

        //Initialize trajectory variables
        next_point_dist = 0;
        goal_dist = 0;
        prev_point_dist = 0;

        goal_reached_ = false;
        controlled_velocity_.steer = 0;
        controlled_velocity_.throttle = 0;
        controlled_velocity_.brake = 0;

        moveit_msgs::RobotTrajectory j;
        traj = j;

        //initialize trajectory variable to plot prediction trajectory
        spline_traj_.poses.resize(100);
        spline_traj2_.poses.resize(100);
        pred_traj_.poses.resize(ACADO_N);
        pred_cmd_.poses.resize(ACADO_N);
        pred_traj_.header.frame_id = controller_config_->target_frame_;
        for(int i=0;i < ACADO_N; i++)
        {
            pred_traj_.poses[i].header.frame_id = controller_config_->target_frame_;
        }

        pred_traj_pub_ = nh.advertise<nav_msgs::Path>("mpc_horizon",1);

        // Initialize pregenerated mpc solver
        acado_initializeSolver( );

        // initialize state and control weight factors
        cost_contour_weight_factors_ = transformStdVectorToEigenVector(controller_config_->contour_weight_factors_);
        cost_control_weight_factors_ = transformStdVectorToEigenVector(controller_config_->control_weight_factors_);
        slack_weight_ = controller_config_->slack_weight_;
        repulsive_weight_ = controller_config_->repulsive_weight_;
        reference_velocity_ = controller_config_->reference_velocity_;
        ini_vel_x_ = controller_config_->ini_vel_x_;
        ros::NodeHandle nh_predictive("predictive_controller");

        /// Setting up dynamic_reconfigure server for the TwistControlerConfig parameters
        reconfigure_server_.reset(new dynamic_reconfigure::Server<lmpcc::PredictiveControllerConfig>(reconfig_mutex_, nh_predictive));
        reconfigure_server_->setCallback(boost::bind(&MPCC::reconfigureCallback,   this, _1, _2));

        // Initialize obstacles
        int N = ACADO_N; // hack.. needs to be beter computed
        obstacles_.Obstacles.resize(controller_config_->n_obstacles_);
        for (int obst_it = 0; obst_it < controller_config_->n_obstacles_; obst_it++)
        {
            obstacles_.Obstacles[obst_it].pose.resize(N);
            obstacles_.Obstacles[obst_it].distance.resize(N);
            obstacles_.Obstacles[obst_it].major_semiaxis.resize(N);
            obstacles_.Obstacles[obst_it].minor_semiaxis.resize(N);
            for(int t = 0;t<N;t++){
                obstacles_.Obstacles[obst_it].pose[t].position.x = 10000;
                obstacles_.Obstacles[obst_it].pose[t].position.y = 10000;
                obstacles_.Obstacles[obst_it].pose[t].orientation.z = 0;
                obstacles_.Obstacles[obst_it].major_semiaxis[t] = 0.001;
                obstacles_.Obstacles[obst_it].minor_semiaxis[t] = 0.001;
            }
        }

        computeEgoDiscs();

        //Controller options
        enable_output_ = false;
        plan_ = false;
        replan_ = false;
        x_offset_= 0;
        y_offset_= 0;
        theta_offset_ = 0;
        debug_ = false;
        n_iterations_ = 100;
        simulation_mode_ = true;

        //Plot variables
        ellips1.type = visualization_msgs::Marker::CYLINDER;
        ellips1.id = 60;
        ellips1.color.b = 1.0;
        ellips1.color.a = 0.2;
        ellips1.header.frame_id = controller_config_->target_frame_;
        ellips1.ns = "trajectory";
        ellips1.action = visualization_msgs::Marker::ADD;
        ellips1.lifetime = ros::Duration(0.1);
        ellips1.scale.x = r_discs_*2.0;
        ellips1.scale.y = r_discs_*2.0;
        ellips1.scale.z = 0.05;

        // Initialize pregenerated mpc solver
        acado_initializeSolver( );

        // MPCC reference path variables
        X_road.resize(2);
        Y_road.resize(2);
        Theta_road.resize(2);

        // Check if all reference vectors are of the same length
        if (!( (controller_config_->ref_x_.size() == controller_config_->ref_y_.size()) && ( controller_config_->ref_x_.size() == controller_config_->ref_theta_.size() ) && (controller_config_->ref_y_.size() == controller_config_->ref_theta_.size()) ))
        {
            ROS_ERROR("Reference path inputs should be of equal length");
        }

        traj_i =0;
        ROS_WARN("PREDICTIVE CONTROL INTIALIZED!!");
        return true;
    }
    else
    {
        ROS_ERROR("MPCC: Failed to initialize as ROS Node is shoutdown");
        return false;
    }
}

void MPCC::computeEgoDiscs()
{
    // Collect parameters for disc representation
    int n_discs = controller_config_->n_discs_;
    double length = controller_config_->ego_l_;
    double width = controller_config_->ego_w_;

    // Initialize positions of discs
    x_discs_.resize(n_discs);

    // Loop over discs and assign positions
    for ( int discs_it = 0; discs_it < n_discs; discs_it++){

        x_discs_[discs_it] = -length/n_discs+(discs_it + 1)*(length/(n_discs));

    }

    // Compute radius of the discs
    r_discs_ = width/2;
    ROS_WARN_STREAM("Generated " << n_discs <<  " ego-vehicle discs with radius " << r_discs_ );
}

void MPCC::broadcastPathPose(){

    geometry_msgs::TransformStamped transformStamped;
    transformStamped.header.stamp = ros::Time::now();
    transformStamped.header.frame_id = controller_config_->target_frame_;
    transformStamped.child_frame_id = "path";

    transformStamped.transform.translation.x = ref_path_x(acadoVariables.x[4]);
    transformStamped.transform.translation.y = ref_path_y(acadoVariables.x[4]);
    transformStamped.transform.translation.z = 0.0;
    tf::Quaternion q = tf::createQuaternionFromRPY(0, 0, pred_traj_.poses[1].pose.orientation.z);
    transformStamped.transform.rotation.x = 0;
    transformStamped.transform.rotation.y = 0;
    transformStamped.transform.rotation.z = 0;
    transformStamped.transform.rotation.w = 1;

    path_pose_pub_.sendTransform(transformStamped);
}

void MPCC::broadcastTF(){
    // Only used for perfect state simulation
    geometry_msgs::TransformStamped transformStamped;
    transformStamped.header.stamp = ros::Time::now();
    transformStamped.header.frame_id = controller_config_->target_frame_;
    transformStamped.child_frame_id = controller_config_->robot_base_link_;
    if(!enable_output_){
        transformStamped.transform.translation.x = current_state_(0);
        transformStamped.transform.translation.y = current_state_(1);
        transformStamped.transform.translation.z = 0.0;
        tf::Quaternion q = tf::createQuaternionFromRPY(0, 0, pred_traj_.poses[1].pose.orientation.z);
        transformStamped.transform.rotation.x = q.x();
        transformStamped.transform.rotation.y = q.y();
        transformStamped.transform.rotation.z = q.z();
        transformStamped.transform.rotation.w = q.w();
    }

    else{
        transformStamped.transform.translation.x = pred_traj_.poses[1].pose.position.x;
        transformStamped.transform.translation.y = pred_traj_.poses[1].pose.position.y;
        transformStamped.transform.translation.z = 0.0;

        tf::Quaternion q = tf::createQuaternionFromRPY(0, 0, pred_traj_.poses[1].pose.orientation.z);
        transformStamped.transform.rotation.x = q.x();
        transformStamped.transform.rotation.y = q.y();
        transformStamped.transform.rotation.z = q.z();
        transformStamped.transform.rotation.w = q.w();
    }

    state_pub_.sendTransform(transformStamped);

    sensor_msgs::JointState empty;
    empty.position.resize(7);
    empty.name ={"rear_right_wheel_joint", "rear_left_wheel_joint", "front_right_wheel_joint", "front_left_wheel_joint","front_right_steer_joint","front_left_steer_joint","steering_joint"};
    empty.header.stamp = ros::Time::now();
    joint_state_pub_.publish(empty);
}

void  MPCC::reset_solver(){
	acadoVariables.dummy = 0;
	int i, j;

	for (i = 0; i < ACADO_N + 1; ++i)
	{
		for (j = 0; j < ACADO_NX; ++j)
			acadoVariables.x[i * ACADO_NX + j]=0;
	}
	for (i = 0; i < ACADO_N; ++i)
	{
		for (j = 0; j < ACADO_NU; ++j){
			acadoVariables.u[i * ACADO_NU + j]=0;
			acadoVariables.mu[i * ACADO_NX + j]=0;
		}
	}
	
	for (j = 0; j < ACADO_NX; ++j)
			acadoVariables.x0[j]=0;
	}

// update this function 1/clock_frequency
void MPCC::runNode(const ros::TimerEvent &event)
{
    int N_iter;

    ROS_ERROR_STREAM("Real-time constraint not satisfied... Cycle time: " << event.profile.last_duration);

    if(!simulation_mode_)
        broadcastTF();
    if (plan_ && (waypoints_size_ > 0)) {

        if (simulation_mode_) {
            acadoVariables.x[0] = current_state_(0);
            acadoVariables.x[1] = current_state_(1);
            acadoVariables.x[2] = current_state_(2);
            acadoVariables.x[3] = current_state_(3)+ini_vel_x_;              //it should be obtained by the wheel speed

            acadoVariables.x0[0] = current_state_(0);
            acadoVariables.x0[1] = current_state_(1);
            acadoVariables.x0[2] = current_state_(2);
            acadoVariables.x0[3] = current_state_(3)+ini_vel_x_;             //it should be obtained by the wheel speed

        } else {
            if (enable_output_) {
                acadoVariables.x[0] = current_state_(0);
                acadoVariables.x[1] = current_state_(1);
                acadoVariables.x[2] = current_state_(2);
                acadoVariables.x[3] = current_state_(3);              //it should be obtained by the wheel speed

                acadoVariables.x0[0] = current_state_(0);
                acadoVariables.x0[1] = current_state_(1);
                acadoVariables.x0[2] = current_state_(2);
                acadoVariables.x0[3] = current_state_(3);                        //it should be obtained by the wheel speed

            } else {
                acadoVariables.x[0] = acadoVariables.x[0];
                acadoVariables.x[1] = acadoVariables.x[1];
                acadoVariables.x[2] = acadoVariables.x[2];
                acadoVariables.x[3] = acadoVariables.x[3];             //it should be obtained by the wheel speed
                acadoVariables.x0[0] = acadoVariables.x[0];
                acadoVariables.x0[1] = acadoVariables.x[1];
                acadoVariables.x0[2] = acadoVariables.x[2];
                acadoVariables.x0[3] = acadoVariables.x[3];             //it should be obtained by the wheel speed
            }
        }
        //ROS_WARN_STREAM("ss.size():" << ss.size() << " traj_i: " << traj_i);
        if (acadoVariables.x[4] > ss[traj_i + 1]) {

            if (traj_i + 2 == ss.size()) {
                goal_reached_ = true;
                ROS_ERROR_STREAM("GOAL REACHED");
            } else {
                traj_i++;
                //ROS_ERROR_STREAM("SWITCH SPLINE " << acadoVariables.x[4]);
            }
        }

        if(enable_output_) {
            double smin;
            smin = spline_closest_point(ss[traj_i], 1000, acadoVariables.x[ACADO_NX+4], window_size_, n_search_points_);
            acadoVariables.x[4] = smin;
            acadoVariables.x0[4] = smin;
            //ROS_ERROR_STREAM("smin: " << smin);
            //ROS_ERROR_STREAM("smin: " << ss[traj_i]);
            //ROS_ERROR_STREAM("smin: " << ss[traj_i+1]);
        }

        acadoVariables.u[0] = controlled_velocity_.throttle;
        acadoVariables.u[1] = controlled_velocity_.steer;
        //acadoVariables.u[2] = 0.0000001;           //slack variable

        for (N_iter = 0; N_iter < ACADO_N; N_iter++) {


            acadoVariables.od[(ACADO_NOD * N_iter) + 0] = ref_path_x.m_a[traj_i];        // spline coefficients
            acadoVariables.od[(ACADO_NOD * N_iter) + 1] = ref_path_x.m_b[traj_i];
            acadoVariables.od[(ACADO_NOD * N_iter) + 2] = ref_path_x.m_c[traj_i];        // spline coefficients
            acadoVariables.od[(ACADO_NOD * N_iter) + 3] = ref_path_x.m_d[traj_i];
            acadoVariables.od[(ACADO_NOD * N_iter) + 4] = ref_path_y.m_a[traj_i];        // spline coefficients
            acadoVariables.od[(ACADO_NOD * N_iter) + 5] = ref_path_y.m_b[traj_i];
            acadoVariables.od[(ACADO_NOD * N_iter) + 6] = ref_path_y.m_c[traj_i];        // spline coefficients
            acadoVariables.od[(ACADO_NOD * N_iter) + 7] = ref_path_y.m_d[traj_i];

            acadoVariables.od[(ACADO_NOD * N_iter) + 16] = ss[traj_i];       // s1
            acadoVariables.od[(ACADO_NOD * N_iter) + 17] = ss[traj_i + 1];       //s2
            acadoVariables.od[(ACADO_NOD * N_iter) + 18] = ss[traj_i + 1] + 0.02;       // d
            acadoVariables.od[(ACADO_NOD * N_iter) + 19] = cost_contour_weight_factors_(0);     // weight contour error
            acadoVariables.od[(ACADO_NOD * N_iter) + 20] = cost_contour_weight_factors_(1);     // weight lag error
            acadoVariables.od[(ACADO_NOD * N_iter) + 21] = cost_control_weight_factors_(0);    // weight acceleration
            acadoVariables.od[(ACADO_NOD * N_iter) + 22] = cost_control_weight_factors_(1);   // weight delta
            acadoVariables.od[(ACADO_NOD * N_iter) + 25] = slack_weight_;                     //slack weight
            acadoVariables.od[(ACADO_NOD * N_iter) + 26] = repulsive_weight_;                     //repulsive weight
            acadoVariables.od[(ACADO_NOD * N_iter) + 39] = velocity_weight_;                     //repulsive weight

            acadoVariables.od[(ACADO_NOD * N_iter) + 27] = r_discs_; //radius of the disks
            acadoVariables.od[(ACADO_NOD * N_iter) + 28] = x_discs_[0];                        // position of the car discs
            acadoVariables.od[(ACADO_NOD * N_iter) + 40] = x_discs_[1];                        // position of the car discs
            acadoVariables.od[(ACADO_NOD * N_iter) + 41] = x_discs_[2];                        // position of the car discs
            //acadoVariables.od[(ACADO_NOD * N_iter) + 42] = x_discs_[3];                        // position of the car discs
            //acadoVariables.od[(ACADO_NOD * N_iter) + 43] = x_discs_[4];

            acadoVariables.od[(ACADO_NOD * N_iter) + 29] = obstacles_.Obstacles[0].pose[N_iter].position.x;      // x position of obstacle 1
            acadoVariables.od[(ACADO_NOD * N_iter) + 30] = obstacles_.Obstacles[0].pose[N_iter].position.y;      // y position of obstacle 1
            //ToDo check convertion from quaternion to RPY angle
            acadoVariables.od[(ACADO_NOD * N_iter) + 31] = obstacles_.Obstacles[0].pose[N_iter].orientation.z;   // heading of obstacle 1
            acadoVariables.od[(ACADO_NOD * N_iter) + 32] = obstacles_.Obstacles[0].major_semiaxis[N_iter];       // major semiaxis of obstacle 1
            acadoVariables.od[(ACADO_NOD * N_iter) + 33] = obstacles_.Obstacles[0].minor_semiaxis[N_iter];       // minor semiaxis of obstacle 1


            acadoVariables.od[(ACADO_NOD * N_iter) + 34] = obstacles_.Obstacles[1].pose[N_iter].position.x;      // x position of obstacle 2
            acadoVariables.od[(ACADO_NOD * N_iter) + 35] = obstacles_.Obstacles[1].pose[N_iter].position.y;      // y position of obstacle 2
            acadoVariables.od[(ACADO_NOD * N_iter) + 36] = obstacles_.Obstacles[1].pose[N_iter].orientation.z;   // heading of obstacle 2
            acadoVariables.od[(ACADO_NOD * N_iter) + 37] = obstacles_.Obstacles[1].major_semiaxis[N_iter];       // major semiaxis of obstacle 2
            acadoVariables.od[(ACADO_NOD * N_iter) + 38] = obstacles_.Obstacles[1].minor_semiaxis[N_iter];       // minor semiaxis of obstacle 2


            if (goal_reached_) {
                reduced_reference_velocity_ = current_state_(3)-4*0.25*N_iter;
                if(reduced_reference_velocity_ < 0)
                    reduced_reference_velocity_=0;

                acadoVariables.od[(ACADO_NOD * N_iter) + 23] = reduced_reference_velocity_;
                acadoVariables.od[(ACADO_NOD * N_iter) + 24] = reduced_reference_velocity_;
            } else {
                acadoVariables.od[(ACADO_NOD * N_iter) + 23] = reference_velocity_;
                acadoVariables.od[(ACADO_NOD * N_iter) + 24] = reference_velocity_;
                acadoVariables.od[(ACADO_NOD * N_iter) + 8] = ref_path_x.m_a[traj_i + 1];        // spline coefficients
                acadoVariables.od[(ACADO_NOD * N_iter) + 9] = ref_path_x.m_b[traj_i + 1];
                acadoVariables.od[(ACADO_NOD * N_iter) + 10] = ref_path_x.m_c[traj_i + 1];        // spline coefficients
                acadoVariables.od[(ACADO_NOD * N_iter) + 11] = ref_path_x.m_d[traj_i + 1];
                acadoVariables.od[(ACADO_NOD * N_iter) + 12] = ref_path_y.m_a[traj_i + 1];        // spline coefficients
                acadoVariables.od[(ACADO_NOD * N_iter) + 13] = ref_path_y.m_b[traj_i + 1];
                acadoVariables.od[(ACADO_NOD * N_iter) + 14] = ref_path_y.m_c[traj_i + 1];        // spline coefficients
                acadoVariables.od[(ACADO_NOD * N_iter) + 15] = ref_path_y.m_d[traj_i + 1];
            }

            //acadoVariables.od[(ACADO_NOD * N_iter) + 23] = ss[traj_i + 1] + 0.02;
        }
        acado_initializeSolver();
        acado_preparationStep();

        acado_feedbackStep();

        //printf("\tReal-Time Iteration:  KKT Tolerance = %.3e\n\n", acado_getKKT());

        int j = 0;
        while (acado_getKKT() > 1e-4 && j < n_iterations_) {

            acado_preparationStep();

            acado_feedbackStep();

            //printf("\tReal-Time Iteration:  KKT Tolerance = %.3e\n\n", acado_getKKT());

            j++;    //        acado_printDifferentialVariables();

            if(j >6){
                for (N_iter = 0; N_iter < ACADO_N; N_iter++) {
                    reduced_reference_velocity_ = current_state_(3) - 2 * 0.25 * N_iter;
                    if(reduced_reference_velocity_ < 0)
                        reduced_reference_velocity_=0;
                    acadoVariables.od[(ACADO_NOD * N_iter) + 23] = reduced_reference_velocity_;
                    acadoVariables.od[(ACADO_NOD * N_iter) + 24] = reduced_reference_velocity_;
                }
            }
            if(reduced_reference_velocity_< reference_velocity_){
                for (N_iter = 0; N_iter < ACADO_N; N_iter++) {
                    reduced_reference_velocity_ = current_state_(3) + 2 * 0.25 * N_iter;
                    if(reduced_reference_velocity_ >reference_velocity_)
                        reduced_reference_velocity_ = reference_velocity_;

                    acadoVariables.od[(ACADO_NOD * N_iter) + 23] = reduced_reference_velocity_;
                    acadoVariables.od[(ACADO_NOD * N_iter) + 24] = reduced_reference_velocity_;
                }
            }
        }

        controlled_velocity_.throttle = acadoVariables.u[0];// / 1.5; // maximum acceleration 1.5m/s
        controlled_velocity_.brake = acado_getKKT();
        controlled_velocity_.steer = acadoVariables.u[1] ;// / 0.52; // maximum steer

        if(debug_){
            te_ = acado_toc(&t);
            //publishPredictedTrajectory();
            //publishPredictedOutput();
            //broadcastPathPose();
            publishFeedback(j,te_);
        }
        publishPredictedCollisionSpace();
        broadcastPathPose();
        brake_.data = controlled_velocity_.brake;
        cost_.data = acado_getObjective();
    }

    if(!enable_output_||acado_getKKT() > 1e-4)
    {
        publishZeroJointVelocity();

    }
    else
        controlled_velocity_pub_.publish(controlled_velocity_);

}

double MPCC::spline_closest_point(double s_min, double s_max, double s_guess, double window, int n_tries){

    double lower = std::max(s_min, s_guess-window);
    double upper = std::min(s_max, s_guess + window);
    double s_i=lower,spline_pos_x_i,spline_pos_y_i;
    double dist_i,min_dist,smin=0.0;

    spline_pos_x_i = ref_path_x(s_i);
    spline_pos_y_i = ref_path_y(s_i);

    min_dist = std::sqrt((spline_pos_x_i-current_state_(0))*(spline_pos_x_i-current_state_(0))+(spline_pos_y_i-current_state_(1))*(spline_pos_y_i-current_state_(1)));

    for(int i=0;i<n_tries;i++){
        s_i = lower+(upper-lower)/n_tries*i;
        spline_pos_x_i = ref_path_x(s_i);
        spline_pos_y_i = ref_path_y(s_i);
        dist_i = std::sqrt((spline_pos_x_i-current_state_(0))*(spline_pos_x_i-current_state_(0))+(spline_pos_y_i-current_state_(1))*(spline_pos_y_i-current_state_(1)));

        if(dist_i<min_dist){
            min_dist = dist_i;
            smin = s_i;
        }

    }
    if(smin < lower){
        smin=lower;
    }
    if(smin > upper){
        smin=upper;
    }

    return smin;

}


void MPCC::Ref_path(std::vector<double> x,std::vector<double> y, std::vector<double> theta) {

    double k, dk, L;
    std::vector<double> X(n_clothoid), Y(n_clothoid);
    std::vector<double> X_all, Y_all, S_all;
    total_length_= 0;
    n_clothoid = controller_config_->n_points_clothoid_;
    n_pts = controller_config_->n_points_spline_;
    S_all.push_back(0);

    for (int i = 0; i < x.size()-1; i++){
        Clothoid::buildClothoid(x[i]+x_offset_, y[i]+y_offset_, theta[i]+theta_offset_, x[i+1]+x_offset_, y[i+1]+y_offset_, theta[i+1]+theta_offset_, k, dk, L);
        Clothoid::pointsOnClothoid(x[i]+x_offset_, y[i]+y_offset_, theta[i]+theta_offset_, k, dk, L, n_clothoid, X, Y);
        if (i==0){
            X_all.insert(X_all.end(), X.begin(), X.end());
            Y_all.insert(Y_all.end(), Y.begin(), Y.end());
        }
        else{
            X.erase(X.begin()+0);
            Y.erase(Y.begin()+0);
            X_all.insert(X_all.end(), X.begin(), X.end());
            Y_all.insert(Y_all.end(), Y.begin(), Y.end());
        }
        total_length_ += L;
        for (int j=1; j< n_clothoid; j++){
            S_all.push_back(S_all[j-1+i*(n_clothoid-1)]+L/(n_clothoid-1));
            //ROS_INFO_STREAM("S_all: " << S_all[j]);
        }
        //ROS_INFO_STREAM("X_all: " << X_all[i]);
        //ROS_INFO_STREAM("Y_all: " << Y_all[i]);
    }

    ref_path_x.set_points(S_all, X_all);
    ref_path_y.set_points(S_all, Y_all);

    dist_spline_pts_ = total_length_ / (n_pts );
    //ROS_INFO_STREAM("dist_spline_pts_: " << dist_spline_pts_);
    ss.resize(n_pts);
    xx.resize(n_pts);
    yy.resize(n_pts);

    for (int i=0; i<n_pts; i++){
        ss[i] = dist_spline_pts_ *i;
        xx[i] = ref_path_x(ss[i]);
        yy[i] = ref_path_y(ss[i]);
        //ROS_INFO_STREAM("ss: " << ss[i]);
        //ROS_INFO_STREAM("xx: " << xx[i]);
        //ROS_INFO_STREAM("yy: " << yy[i]);
        path_length_ = ss[i];
    }

    ref_path_x.set_points(ss,xx);
    ref_path_y.set_points(ss,yy);
}

void MPCC::ConstructRefPath(){
    geometry_msgs::Pose pose;
    double ysqr, t3, t4;
    tf2::Quaternion myQuaternion;
    for (int ref_point_it = 0; ref_point_it < controller_config_->ref_x_.size(); ref_point_it++)
    {
        pose.position.x = controller_config_->ref_x_.at(ref_point_it);
        pose.position.y = controller_config_->ref_y_.at(ref_point_it);
        // Convert RPY from path to quaternion
        myQuaternion.setRPY( 0, 0, controller_config_->ref_theta_.at(ref_point_it) );
        pose.orientation.x = myQuaternion.x();
        pose.orientation.y = myQuaternion.y();
        pose.orientation.z = myQuaternion.z();
        pose.orientation.w = myQuaternion.w();
        // Convert from global_frame to planning frame
        //transformPose(controller_config_->global_path_frame_,controller_config_->target_frame_,pose);
        X_road[ref_point_it] = pose.position.x;
        Y_road[ref_point_it] = pose.position.y;
        // Convert from quaternion to RPY
        ysqr = pose.orientation.y * pose.orientation.y;
        t3 = +2.0 * (pose.orientation.w * pose.orientation.z
                             + pose.orientation.x *pose.orientation.y);
        t4 = +1.0 - 2.0 * (ysqr + pose.orientation.z * pose.orientation.z);
        Theta_road[ref_point_it] = std::atan2(t3, t4);
    }

    Ref_path(X_road, Y_road, Theta_road);
}

void MPCC::getWayPointsCallBack(nav_msgs::Path waypoints){
	//
	last_waypoints_size_ = waypoints_size_;
	if (waypoints.poses.size()==0){
		
		
		ROS_WARN("Waypoint message is empty");
		X_road[0] = 0.0;
		Y_road[0] = 0.0;
		Theta_road[0] = 0.0; //to do conversion quaternion
			
		X_road[1] = 300.0;
		Y_road[1] = 0.0;
		Theta_road[1] = 0.0; //to do conversion quaternion
			
        waypoints_size_ = 2.0;
           
	}
    else
	{
	    ROS_INFO("Getting waypoints from SOMEWHERE...");
        waypoints_size_ = waypoints.poses.size();
 		for (int ref_point_it = 0; ref_point_it<waypoints_size_; ref_point_it++)
        {
		    X_road[ref_point_it] = waypoints.poses.at(ref_point_it).pose.position.x;
		    Y_road[ref_point_it] = waypoints.poses.at(ref_point_it).pose.position.y;
		    Theta_road[ref_point_it] = quaternionToangle(waypoints.poses.at(ref_point_it).pose.orientation); //to do conversion quaternion
        }
	}
	  
    //ConstructRefPath();
    Ref_path(X_road, Y_road, Theta_road);
    //ROS_INFO("ConstructRefPath");
    publishSplineTrajectory();
    plotRoad();
}

double MPCC::quaternionToangle(geometry_msgs::Quaternion q){

  double ysqr, t3, t4;
  
  // Convert from quaternion to RPY
  ysqr = q.y * q.y;
  t3 = +2.0 * (q.w *q.z + q.x *q.y);
  t4 = +1.0 - 2.0 * (ysqr + q.z * q.z);
  return std::atan2(t3, t4);
}

void MPCC::reconfigureCallback(lmpcc::PredictiveControllerConfig& config, uint32_t level){

    ROS_INFO("reconfigure callback!");
    cost_contour_weight_factors_(0) = config.Wcontour;
    cost_contour_weight_factors_(1) = config.Wlag;
    cost_control_weight_factors_(0) = config.Ka;
    cost_control_weight_factors_(1) = config.Kdelta;
    velocity_weight_ = config.Kv;
    ini_vel_x_ = config.ini_v0;

    slack_weight_= config.Ws;
    repulsive_weight_ = config.WR;

    reference_velocity_ = config.vRef;
    slack_weight_= config.Ws;
    repulsive_weight_ = config.WR;

    n_iterations_ = config.n_iterations;
    simulation_mode_ = config.simulation_mode;

    //Search window parameters
    window_size_ = config.window_size;
    n_search_points_ = config.n_search_points;

    if(x_offset_!=config.x_offset) {
        x_offset_ = config.x_offset;
        replan_ = true;
    }
    if(y_offset_!=config.y_offset) {
        y_offset_ = config.y_offset;
        replan_ = true;
    }
    if(theta_offset_!=config.theta_offset) {
        theta_offset_ = config.theta_offset;
        replan_ = true;
    }
    if(replan_ && waypoints_size_>0){

        Ref_path(X_road, Y_road, Theta_road);
        //ROS_INFO("ConstructRefPath");
        publishSplineTrajectory();
        plotRoad();
        replan_ = false;
    }
    /*if(replan_){
        geometry_msgs::Pose pose;

        pose.position.x = x_offset_;
        pose.position.y = 0;
        // Convert RPY from path to quaternion

        pose.orientation.x = 0;
        pose.orientation.y = 0;
        pose.orientation.z = 0;
        pose.orientation.w = 1;
            // Convert from global_frame to planning frame
        transformPose(controller_config_->robot_base_link_,controller_config_->target_frame_,pose);

        for (int ref_point_it = 0; ref_point_it<waypoints_size_; ref_point_it++)
        {
            X_road[ref_point_it] = pose.position.x;
            Y_road[ref_point_it] = pose.position.y;
            Theta_road[ref_point_it] = quaternionToangle(pose.orientation); //to do conversion quaternion
        }

        Ref_path(X_road, Y_road, Theta_road);
        //ROS_INFO("ConstructRefPath");
        publishSplineTrajectory();
    }
    */
    debug_ = config.debug;
    if (waypoints_size_ !=0) {
        plan_ = config.plan;
        enable_output_ = config.enable_output;
    } else {
        config.plan = false;
        config.enable_output = false;
        plan_ = false;        
        enable_output_ = false;
    }

    if(plan_){
		reset_solver();
		
		acado_initializeSolver( );
		//ROS_INFO("acado_initializeSolver");
		//ConstructRefPath();
		//getWayPointsCallBack( );
		//ROS_INFO("ConstructRefPath");
		//Ref_path(X_road, Y_road, Theta_road);
        publishSplineTrajectory();
        //ROS_INFO("reconfigure callback!");
        traj_i = 0;
        goal_reached_ = false;
        timer_.start();
	}
    else{
        timer_.stop();
    }
}

void MPCC::executeTrajectory(const moveit_msgs::RobotTrajectory & traj){

}

// read current position and velocity of robot joints
void MPCC::StateCallBack(const nav_msgs::Odometry::ConstPtr& msg)
{
   double ysqr, t3, t4;
   if (activate_debug_output_)
   {
//  ROS_INFO("MPCC::StateCallBack");
   }
   //ROS_INFO("MPCC::StateCallBack");
   controller_config_->target_frame_ = msg->header.frame_id;
   last_state_ = current_state_;
   current_state_(0) =    msg->pose.pose.position.x;
   current_state_(1) =    msg->pose.pose.position.y;
   ysqr = msg->pose.pose.orientation.y * msg->pose.pose.orientation.y;
   t3 = +2.0 * (msg->pose.pose.orientation.w * msg->pose.pose.orientation.z
                             + msg->pose.pose.orientation.x *msg->pose.pose.orientation.y);
   t4 = +1.0 - 2.0 * (ysqr + msg->pose.pose.orientation.z * msg->pose.pose.orientation.z);

   current_state_(2) = std::atan2(t3, t4);

   current_state_(3) = std::sqrt(std::pow(msg->twist.twist.linear.x,2)+std::pow(msg->twist.twist.linear.y,2));
}
void MPCC::ObstacleStateCallback(const cv_msgs::PredictedMoGTracks& objects)
{
    double ysqr, t3, t4;
    //ROS_INFO_STREAM("N tracks: "<<objects.tracks.size());
    if(objects.tracks.size()) {
        //ROS_INFO_STREAM("N track: " << objects.tracks[0].track.size());
        //ROS_INFO_STREAM("N: " << objects.tracks[0].track[0].pose.size());
    }
    //reset all objects
    for (int obst_it = 0; obst_it < controller_config_->n_obstacles_; obst_it++)
    {
        for(int t = 0;t<obstacles_.Obstacles[obst_it].pose.size();t++){
            obstacles_.Obstacles[obst_it].pose[t].position.x = 10000;
            obstacles_.Obstacles[obst_it].pose[t].position.y = 10000;
            obstacles_.Obstacles[obst_it].pose[t].orientation.z = 0;
            obstacles_.Obstacles[obst_it].major_semiaxis[t] = 0.001;
            obstacles_.Obstacles[obst_it].minor_semiaxis[t] = 0.001;
        }
    }
    for(int i=0;i<objects.tracks.size();i++){
        cv_msgs::PredictedMoGTrack track = objects.tracks[i];
        for(int j=0;j<track.track.size();j++){
            cv_msgs::PredictedMoG mog = track.track[j];
            int mogs_to_consider = 1; //mog.pose.size(); //FIXME only use the first item in the MOG, to allow for multi-object tracking.
            for (int k = 0; k < mogs_to_consider; k++) {
                int current_obstacle = i*mogs_to_consider + k;
                if (j == 0) {
                  std::cout << "current obstacle: " << current_obstacle << std::endl;
                  std::cout << "current i: " << i << std::endl;
                }
                obstacles_.Obstacles[current_obstacle].pose[j]=mog.pose[k].pose;
                //ToDo
                transformPose(objects.header.frame_id,controller_config_->target_frame_,obstacles_.Obstacles[current_obstacle].pose[j]);

                // Convert quaternion to RPY
                ysqr = obstacles_.Obstacles[current_obstacle].pose[j].orientation.y * obstacles_.Obstacles[current_obstacle].pose[j].orientation.y;
                t3 = +2.0 * (obstacles_.Obstacles[current_obstacle].pose[j].orientation.w * obstacles_.Obstacles[current_obstacle].pose[j].orientation.z
                             + obstacles_.Obstacles[current_obstacle].pose[j].orientation.x * obstacles_.Obstacles[current_obstacle].pose[j].orientation.y);
                t4 = +1.0 - 2.0 * (ysqr + obstacles_.Obstacles[current_obstacle].pose[j].orientation.z * obstacles_.Obstacles[current_obstacle].pose[j].orientation.z);

                obstacles_.Obstacles[current_obstacle].pose[j].orientation.z = std::atan2(t3, t4);

                obstacles_.Obstacles[current_obstacle].major_semiaxis[j] = 0.5;
                obstacles_.Obstacles[current_obstacle].minor_semiaxis[j] = 0.5;

            }
        }
    }
  for(int i=0;i<objects.tracks.size();i++){
    cv_msgs::PredictedMoGTrack track = objects.tracks[i];
    for(int j=track.track.size();j<ACADO_N;j++){
      cv_msgs::PredictedMoG mog = track.track[track.track.size()-1];
      for (int k = 0; k < mog.pose.size(); k++) {
        obstacles_.Obstacles[k].pose[j]=mog.pose[k].pose;
        //ToDo
        transformPose(objects.header.frame_id,controller_config_->target_frame_,obstacles_.Obstacles[k].pose[j]);

        // Convert quaternion to RPY
        ysqr = obstacles_.Obstacles[k].pose[j].orientation.y * obstacles_.Obstacles[k].pose[j].orientation.y;
        t3 = +2.0 * (obstacles_.Obstacles[k].pose[j].orientation.w * obstacles_.Obstacles[k].pose[j].orientation.z
                     + obstacles_.Obstacles[k].pose[j].orientation.x * obstacles_.Obstacles[k].pose[j].orientation.y);
        t4 = +1.0 - 2.0 * (ysqr + obstacles_.Obstacles[k].pose[j].orientation.z * obstacles_.Obstacles[k].pose[j].orientation.z);

        obstacles_.Obstacles[k].pose[j].orientation.z = std::atan2(t3, t4);

        obstacles_.Obstacles[k].major_semiaxis[j] = 0.5;
        obstacles_.Obstacles[k].minor_semiaxis[j] = 0.5;

      }
    }
  }
}
void MPCC::ObstacleCallBack(const obstacle_feed::Obstacles& obstacles)
{

    obstacle_feed::Obstacles total_obstacles;
    total_obstacles.Obstacles.resize(controller_config_->n_obstacles_);

    total_obstacles.Obstacles = obstacles.Obstacles;

//    ROS_INFO_STREAM("-- Received # obstacles: " << obstacles.Obstacles.size());
//    ROS_INFO_STREAM("-- Expected # obstacles: " << controller_config_->n_obstacles_);

    if (obstacles.Obstacles.size() < controller_config_->n_obstacles_)
    {
        int N = controller_config_->end_time_horizon_/controller_config_->sampling_time_;

        for (int obst_it = obstacles.Obstacles.size(); obst_it < controller_config_->n_obstacles_; obst_it++)
        {
            total_obstacles.Obstacles[obst_it].pose.resize(N);
            total_obstacles.Obstacles[obst_it].distance.resize(N);
            total_obstacles.Obstacles[obst_it].major_semiaxis.resize(N);
            total_obstacles.Obstacles[obst_it].minor_semiaxis.resize(N);
            for(int t = 0;t<N;t++) {
                total_obstacles.Obstacles[obst_it].pose[t].position.x = 10000;
                total_obstacles.Obstacles[obst_it].pose[t].position.y = 10000;
                total_obstacles.Obstacles[obst_it].pose[t].orientation.z = 0;
                total_obstacles.Obstacles[obst_it].major_semiaxis[t] = 0.001;
                total_obstacles.Obstacles[obst_it].minor_semiaxis[t] = 0.001;
            }
        }
    }

    obstacles_.Obstacles.resize(controller_config_->n_obstacles_);

    for (int total_obst_it = 0; total_obst_it < controller_config_->n_obstacles_; total_obst_it++)
    {
        obstacles_.Obstacles[total_obst_it] = total_obstacles.Obstacles[total_obst_it];
    }

//    ROS_INFO_STREAM("-- total_Obst1: [" << total_obstacles.Obstacles[0].pose.position.x << ",  " << total_obstacles.Obstacles[0].pose.position.y << "], Obst2 [" << total_obstacles.Obstacles[1].pose.position.x << ",  " << total_obstacles.Obstacles[1].pose.position.y << "]");
//    ROS_INFO_STREAM("-- Obst1_: [" << obstacles_.Obstacles[0].pose.position.x << ",  " << obstacles_.Obstacles[0].pose.position.y << "], Obst2 [" << obstacles_.Obstacles[1].pose.position.x << ",  " << obstacles_.Obstacles[1].pose.position.y << "]");
}

void MPCC::publishZeroJointVelocity()
{
    if (activate_debug_output_)
    {
//        ROS_INFO("Publishing ZERO joint velocity!!");
    }
    lmpcc::Control pub_msg;
    if(!simulation_mode_)
        broadcastTF();
    controlled_velocity_ = pub_msg;
    controlled_velocity_.throttle = -0.3;
    controlled_velocity_.steer = 0.0;
    controlled_velocity_pub_.publish(controlled_velocity_);
}

void MPCC::plotRoad(void)
{
    visualization_msgs::Marker line_strip;
    visualization_msgs::MarkerArray line_list;
    line_strip.header.frame_id = controller_config_->target_frame_;
    line_strip.id = 1;

    line_strip.type = visualization_msgs::Marker::LINE_STRIP;

    // LINE_STRIP/LINE_LIST markers use only the x component of scale, for the line width
    line_strip.scale.x = 0.5;
    line_strip.scale.y = 0.5;

    // Line strip is blue
    line_strip.color.b = 1.0;
    line_strip.color.a = 1.0;

    geometry_msgs::Pose pose;

    pose.position.x = (controller_config_->road_width_left_+line_strip.scale.x/2.0)*-sin(current_state_(2));
    pose.position.y = (controller_config_->road_width_left_+line_strip.scale.x/2.0)*cos(current_state_(2));

    geometry_msgs::Point p;
    p.x = spline_traj_.poses[0].pose.position.x + pose.position.x;
    p.y = spline_traj_.poses[0].pose.position.y + pose.position.y;
    p.z = 0.2;  //z a little bit above ground to draw it above the pointcloud.

    line_strip.points.push_back(p);

    p.x = spline_traj_.poses[spline_traj_.poses.size()-1].pose.position.x+ pose.position.x;
    p.y = spline_traj_.poses[spline_traj_.poses.size()-1].pose.position.y+ pose.position.y;
    p.z = 0.2;  //z a little bit above ground to draw it above the pointcloud.

    line_strip.points.push_back(p);

    line_list.markers.push_back(line_strip);

    line_strip.points.pop_back();
    line_strip.points.pop_back();

    line_strip.color.b = 1.0;
    line_strip.color.a = 1.0;
    line_strip.id = 2;
    pose.position.x = (-controller_config_->road_width_right_-line_strip.scale.x/2.0)*-sin(current_state_(2));
    pose.position.y = (-controller_config_->road_width_right_-line_strip.scale.x/2.0)*cos(current_state_(2));

    p.x = spline_traj_.poses[0].pose.position.x+pose.position.x;
    p.y = spline_traj_.poses[0].pose.position.y+pose.position.y;
    p.z = 0.2;  //z a little bit above ground to draw it above the pointcloud.

    line_strip.points.push_back(p);

    p.x = spline_traj_.poses[spline_traj_.poses.size()-1].pose.position.x+pose.position.x;
    p.y = spline_traj_.poses[spline_traj_.poses.size()-1].pose.position.y+pose.position.y;
    p.z = 0.2;  //z a little bit above ground to draw it above the pointcloud.

    line_strip.points.push_back(p);

    line_list.markers.push_back(line_strip);

    marker_pub_.publish(line_list);

}

void MPCC::publishSplineTrajectory(void)
{
    spline_traj_.header.stamp = ros::Time::now();
    spline_traj_.header.frame_id = controller_config_->target_frame_;
    for (int i = 0; i < spline_traj_.poses.size(); i++) // 100 points
    {
        spline_traj_.poses[i].pose.position.x = ref_path_x(i*(n_pts)*dist_spline_pts_/spline_traj_.poses.size()); //x
        spline_traj_.poses[i].pose.position.y = ref_path_y(i*(n_pts)*dist_spline_pts_/spline_traj_.poses.size()); //y
        spline_traj_.poses[i].pose.position.z = 0.2; //z a little bit above ground to draw it above the pointcloud.
    }

    spline_traj_pub_.publish(spline_traj_);
}

void MPCC::publishPredictedTrajectory(void)
{
    for (int i = 0; i < ACADO_N; i++)
    {
        pred_traj_.poses[i].pose.position.x = acadoVariables.x[i * ACADO_NX + 0]; //x
        pred_traj_.poses[i].pose.position.y = acadoVariables.x[i * ACADO_NX + 1]; //y
        pred_traj_.poses[i].pose.orientation.z = acadoVariables.x[i * ACADO_NX + 2]; //theta
    }

    pred_traj_pub_.publish(pred_traj_);
}

void MPCC::publishPredictedOutput(void)
{
    for (int i = 0; i < ACADO_N; i++)
    {
        pred_cmd_.poses[i].pose.position.x = acadoVariables.u[i*ACADO_NU + 0]; //acc
       
		
        pred_cmd_.poses[i].pose.position.y = acadoVariables.u[i*ACADO_NU + 1]; //delta
        pred_cmd_.poses[i].pose.position.z = acadoVariables.u[i*ACADO_NU+ 2];  //slack
    }

    pred_cmd_pub_.publish(pred_cmd_);
}

void MPCC::publishPredictedCollisionSpace(void)
{
    visualization_msgs::MarkerArray collision_space;

    for (int k = 0; k< controller_config_->n_discs_; k++){

        for (int i = 0; i < ACADO_N; i++)
        {
            ellips1.id = 60+i+k*ACADO_N;
            ellips1.pose.position.x = acadoVariables.x[i * ACADO_NX + 0]+x_discs_[k]*cos(acadoVariables.x[i * ACADO_NX + 2]);
            ellips1.pose.position.y = acadoVariables.x[i * ACADO_NX + 1]+x_discs_[k]*sin(acadoVariables.x[i * ACADO_NX + 2]);
            ellips1.pose.position.z = 0.2;  //z a little bit above ground to draw it above the pointcloud.
            ellips1.pose.orientation.x = 0;
            ellips1.pose.orientation.y = 0;
            ellips1.pose.orientation.z = 0;
            ellips1.pose.orientation.w = 1;
            collision_space.markers.push_back(ellips1);
        }
    }
    robot_collision_space_pub_.publish(collision_space);
}

void MPCC::publishCost(void){

    cost_pub_.publish(cost_);
    brake_pub_.publish(brake_);
}

void MPCC::publishFeedback(int& it, double& time)
{

    lmpcc::control_feedback feedback_msg;

    feedback_msg.header.stamp = ros::Time::now();
    feedback_msg.header.frame_id = controller_config_->target_frame_;

    feedback_msg.cost = cost_.data;
    feedback_msg.iterations = it;
    feedback_msg.computation_time = time;
    feedback_msg.kkt = acado_getKKT();

    feedback_msg.wC = cost_contour_weight_factors_(0);       // weight factor on contour error
    feedback_msg.wL = cost_contour_weight_factors_(1);       // weight factor on lag error
    feedback_msg.wV = cost_control_weight_factors_(0);       // weight factor on theta
    feedback_msg.wW = cost_control_weight_factors_(1);

    // Compute contour errors
    feedback_msg.contour_errors.data.resize(2);

    feedback_msg.contour_errors.data[0] = contour_error_;
    feedback_msg.contour_errors.data[1] = lag_error_;

    feedback_msg.reference_path = spline_traj2_;
    feedback_msg.prediction_horizon = pred_traj_;
    feedback_msg.prediction_horizon.poses[0].pose.position.z = acadoVariables.x[3];

    //Search window parameters
    feedback_msg.window = window_size_;
    feedback_msg.search_points = n_search_points_;

    feedback_pub_.publish(feedback_msg);
}

// Utils

bool MPCC::transformPose(const std::string& from, const std::string& to, geometry_msgs::Pose& pose)
{
    bool transform = false;
    tf::StampedTransform stamped_tf;
    //ROS_INFO_STREAM("Transforming from :" << from << " to: " << to);
    geometry_msgs::PoseStamped stampedPose_in, stampedPose_out;

    stampedPose_in.pose = pose;
    if(std::sqrt(std::pow(pose.orientation.x,2)+std::pow(pose.orientation.y,2)+std::pow(pose.orientation.z,2)+std::pow(pose.orientation.w,2))<1){
        stampedPose_in.pose.orientation.x = 0;
        stampedPose_in.pose.orientation.y = 0;
        stampedPose_in.pose.orientation.z = 0;
        stampedPose_in.pose.orientation.w = 1;
    }
//    stampedPose_in.header.stamp = ros::Time::now();
    stampedPose_in.header.frame_id = from;

    // make sure source and target frame exist
    if (tf_listener_.frameExists(to) && tf_listener_.frameExists(from))
    {
        try
        {
            // find transforamtion between souce and target frame
            tf_listener_.waitForTransform(from, to, ros::Time(0), ros::Duration(0.02));
            tf_listener_.transformPose(to, stampedPose_in, stampedPose_out);

            transform = true;
        }
        catch (tf::TransformException& ex)
        {
            ROS_ERROR("MPCC::getTransform: %s", ex.what());
        }
    }

    else
    {
        ROS_WARN("MPCC::getTransform: '%s' or '%s' frame doesn't exist, pass existing frame",from.c_str(), to.c_str());
    }
    pose = stampedPose_out.pose;
    stampedPose_in.pose = stampedPose_out.pose;
    stampedPose_in.header.frame_id = to;

    return transform;
}
