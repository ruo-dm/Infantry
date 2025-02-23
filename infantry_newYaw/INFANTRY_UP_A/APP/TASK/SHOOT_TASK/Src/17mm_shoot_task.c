#include "17mm_shoot_task.h"

//#define FRIC_SPEED_R                      1050
//#define FRIC_SPEED_L                     -1050
//#define POCK_SPEED                       -400
#define BULLET_SPEED_TARGET               27.5
#define BULLET_SPEED_SELF_ADAPTATION_K    50

/* Variables_definination-----------------------------------------------------------------------------------------------*/
shoot_t shoot ={
        .Bullet_Speed_Kalman.X_hat=27,
        .Bullet_Speed_Kalman.Error_Mea=0.2,
        .Bullet_Speed_Kalman.Error_Est=5,
};

	pid_t pid_trigger_angle[4] ={0};
	pid_t pid_trigger_angle_buf={0};
	pid_t pid_trigger_speed_buf={0};
/*----------------------------------------------------------------------------------------------------------------------*/

	
/**
 * @brief 卡尔曼滤波计算 
 * */
float First_Order_Kalman_Filter_Cal(First_Order_Kalman_Filter_t *_First_Order_Kalman_Filter,float _Z/*测量值*/)
{
        //更新上一次的值
        _First_Order_Kalman_Filter->Error_Est_Last=_First_Order_Kalman_Filter->Error_Est;
        _First_Order_Kalman_Filter->X_hat_Last=_First_Order_Kalman_Filter->X_hat;
        //更新上一次的值
        _First_Order_Kalman_Filter->Error_Est_Last=_First_Order_Kalman_Filter->Error_Est;
        _First_Order_Kalman_Filter->X_hat_Last=_First_Order_Kalman_Filter->X_hat;
        //Step1:Kalman_Gain计算
        _First_Order_Kalman_Filter->Kalman_Gain=
        _First_Order_Kalman_Filter->Error_Est_Last
        /(_First_Order_Kalman_Filter->Error_Est_Last+_First_Order_Kalman_Filter->Error_Mea);
        //Step2:计算预测值
        _First_Order_Kalman_Filter->X_hat=
        _First_Order_Kalman_Filter->X_hat_Last
        +_First_Order_Kalman_Filter->Kalman_Gain*(_Z-_First_Order_Kalman_Filter->X_hat_Last);
        //Step3:预测误差更新
        _First_Order_Kalman_Filter->Error_Est=(1-_First_Order_Kalman_Filter->Kalman_Gain)
        *_First_Order_Kalman_Filter->Error_Est_Last;
        //返回预测值
        return _First_Order_Kalman_Filter->X_hat;
}


/**
 * @brief  弹速自适应实现函数
 * */
float Shooter_Bullet_Speed_Self_Adaptation(float Bullet_Speed)
{
        float static Bullet_Speed_Error;
        Bullet_Speed_Error=(BULLET_SPEED_TARGET-Bullet_Speed);
        
        return Bullet_Speed_Error*BULLET_SPEED_SELF_ADAPTATION_K;
}



#if SHOOT_TYPE == 3//发射参数初始化
void shot_param_init()
{

  PID_struct_init(&pid_trigger_angle[0], POSITION_PID, 6000, 1000,20,0.2,0);
	PID_struct_init(&pid_trigger_speed[0],POSITION_PID,10000,10000,30,0,0);//150
	PID_struct_init(&pid_trigger_speed[1],POSITION_PID,19000,10000,50,0.1,4);
	
	PID_struct_init(&pid_trigger_angle_buf,POSITION_PID, 2000 , 50    ,  16, 0.01f  , 0);
	PID_struct_init(&pid_trigger_speed_buf,POSITION_PID,10000 , 5500 ,  80 , 0  , 0 );
	
  PID_struct_init(&pid_rotate[1], POSITION_PID,15500,11500,50,0,0);
  PID_struct_init(&pid_rotate[0], POSITION_PID,15500,11500,50,0,0);

  shoot.friction_pid.speed_ref[0] =FRICTION_SPEED_30;            //没装裁判系统，因此先进行摩擦轮速度赋值
    shoot.shoot_frequency=20;
  shoot.ctrl_mode=1;
  shoot.limit_heart0=80;
}

#endif


#if SHOOT_TYPE == 3//步兵发射模式选择


void heat_switch()
{
											//+\0.002*shoot.cooling_ratio
	//剩余发弹量=（热量上限-热量值）/10      ————一个弹丸的热量为10
  shoot.remain_bullets=(judge_rece_mesg.game_robot_state.shooter_barrel_heat_limit-\
										 judge_rece_mesg.power_heat_data.shooter_id1_17mm_cooling_heat)/10;    //剩余发射量
  shoot.cooling_ratio=judge_rece_mesg.game_robot_state.shooter_barrel_cooling_value;      //冷却速率
      
	//计算射频
	if(shoot.bulletspead_level==1)
				{
					shoot.shoot_frequency=22;
				}
	else{
			if(judge_rece_mesg.game_robot_state.robot_level==1)//level_冷却
				shoot.shoot_frequency=14;						
			else if(judge_rece_mesg.game_robot_state.robot_level==2)//level_2
				shoot.shoot_frequency=16;
			else if(judge_rece_mesg.game_robot_state.robot_level==3)//level_3
				shoot.shoot_frequency=18;
			else if(judge_rece_mesg.game_robot_state.robot_level==4)
				shoot.shoot_frequency=19;
			else if(judge_rece_mesg.game_robot_state.robot_level==5)
				shoot.shoot_frequency=20;
			else if(judge_rece_mesg.game_robot_state.robot_level==6)
				shoot.shoot_frequency=21;
			else if(judge_rece_mesg.game_robot_state.robot_level==7)
				shoot.shoot_frequency=24;
			else if(judge_rece_mesg.game_robot_state.robot_level==8)
				shoot.shoot_frequency=25;
			else if(judge_rece_mesg.game_robot_state.robot_level==9)
				shoot.shoot_frequency=25;
			else if(judge_rece_mesg.game_robot_state.robot_level==10)
				shoot.shoot_frequency=26;
			else
				shoot.shoot_frequency=19;					
		 }

							
}
#endif

#if SHOOT_TYPE == 3//发射热量限制
void heat_shoot_frequency_limit()//步兵射频限制部分
{

		heat_switch();//热量模式选择
//		bullets_spilling();//泼弹

	if(shoot.shoot_frequency!=0)                          //如果射频不为0（射频用来设定热量限制）
	{
		//计算2ms一次  time_tick 1ms更新一次
		
		switch(judge_rece_mesg.game_robot_state.robot_level)
		{
			case 1:
			{shoot.will_time_shoot=(shoot.remain_bullets-2.7)*1000/shoot.shoot_frequency;}break;
			case 2:
			{shoot.will_time_shoot=(shoot.remain_bullets-2.7)*1000/shoot.shoot_frequency;}break;
			case 3:
			{shoot.will_time_shoot=(shoot.remain_bullets-4)*1000/shoot.shoot_frequency;}break;
			case 4:
			{shoot.will_time_shoot=(shoot.remain_bullets-4.2)*1000/shoot.shoot_frequency;}break;
			case 5:
			{shoot.will_time_shoot=(shoot.remain_bullets-4.4)*1000/shoot.shoot_frequency;}break;
			case 6:
			{shoot.will_time_shoot=(shoot.remain_bullets-4.6)*1000/shoot.shoot_frequency;}break;
			case 7:
			{shoot.will_time_shoot=(shoot.remain_bullets-5.0)*1000/shoot.shoot_frequency;}break;
			case 8:
			{shoot.will_time_shoot=(shoot.remain_bullets-5.8)*1000/shoot.shoot_frequency;}break;
			case 9:
			{shoot.will_time_shoot=(shoot.remain_bullets-6.8)*1000/shoot.shoot_frequency;}break;
			case 10:
			{shoot.will_time_shoot=(shoot.remain_bullets-7.4)*1000/shoot.shoot_frequency;}break;
			default :
			{shoot.will_time_shoot=(shoot.remain_bullets-4)*1000/shoot.shoot_frequency;} break;
			
		}
//			 if(shoot.shoot_frequency>20)                   //高射速时，计算射击时间
//			 {
//				 shoot.will_time_shoot=(shoot.remain_bullets-4)*1000/shoot.shoot_frequency;             //没有裁判系统就读不到剩余弹量，就没法发射
//			 }
//			 else if(shoot.shoot_frequency>13)
//			 {
//				 shoot.will_time_shoot=(shoot.remain_bullets-3.5)*1000/shoot.shoot_frequency;//3.5
//			 }
//			 else
//			 {
//				 shoot.will_time_shoot=(shoot.remain_bullets-1.5)*1000/shoot.shoot_frequency;
//			 }
	}
}	
	
void bullets_spilling()//步兵射频限制部分
{
	static 	int fric_run_time=0;
	
	if(shoot.fric_wheel_run==1&&
					 shoot.poke_run==1&&
					 shoot.ctrl_mode==1)
	{ 
		fric_run_time++;
		if(fric_run_time<20&&shoot.remain_bullets>4)           //刚开始发弹且剩余量很多时可以提高射频     
		{shoot.shoot_frequency=shoot.shoot_frequency*1.2;}	
	}
	else
	{
		fric_run_time=0;
		if(shoot.shoot_frequency!=0)
		{pid_trigger_angle[0].set=pid_trigger_angle[0].get;}//松手立即停     
//		shoot.shoot_frequency=0;
	}	

}
void heat0_limit(void)           //热量限制
{  //由于发送的数据为50hz，而且每次进入该操作需要5次循环，因此计算时 公式为：冷却上限-热量值+0.1*冷却值
  shoot.limit_heart0 = judge_rece_mesg.game_robot_state.shooter_barrel_heat_limit-judge_rece_mesg.power_heat_data.shooter_id1_17mm_cooling_heat+0.002*judge_rece_mesg.game_robot_state.shooter_barrel_cooling_value;
	if(shoot.limit_heart0>15)//residual_heat)
	{shoot.ctrl_mode=1;}
  else
  {shoot.ctrl_mode=0;}
}
#endif


#if SHOOT_TYPE == 3//拨盘 trigger poke
u8 rotate_flag=0;
u8 reverse_once_flag=0;
uint32_t poke_cnt=0;
uint32_t poke_lock_cnt=0;
uint8_t poke_reach_flag = 0;
uint8_t poke_lock_flag = 0;
uint8_t buff_poke_init = 0;

uint32_t start_shooting_count = 0;//正转计时
uint32_t start_reversal_count1 = 0;//反转计时
uint8_t lock_rotor1 = 0;//堵转标志位
u8  press_l_flag;
void shoot_bullet_handle(void)
{	

	shoot.single_angle=45;

	heat_shoot_frequency_limit();//步兵射频限制

	
	if(gimbal_data.ctrl_mode!=GIMBAL_AUTO_SMALL_BUFF&&
		 gimbal_data.ctrl_mode!=GIMBAL_AUTO_BIG_BUFF)//正常模式   
	  {//热量限制
		  if(shoot.will_time_shoot>0&&
				 shoot.fric_wheel_run==1&&
							 shoot.poke_run==1&&				
						  shoot.ctrl_mode==1)
//			  if(shoot.fric_wheel_run==1&&
//							 shoot.poke_run==1)	
		{	
/**/
			start_shooting_count++;
			if((start_shooting_count >= 250)&&(abs(general_poke.poke.filter_rate) < 3))
			{
				lock_rotor1 = 1;
				start_shooting_count = 0;
			}
			if(lock_rotor1 == 1)
			{
				start_reversal_count1++;
			if(start_reversal_count1 > 150)
			{
				lock_rotor1 = 0;
				start_reversal_count1 = 0;
			}
			shoot.poke_pid.angle_ref[0]=shoot.poke_pid.angle_fdb[0]-shoot.shoot_frequency*45*36/500;		//一秒shoot_frequency发，一发拨盘转45°，减速比是1：36。每两毫秒执行一次故除以500。	
			shoot.poke_pid.angle_fdb[0]=general_poke.poke.ecd_angle;
			shoot.poke_pid.speed_fdb[0]=general_poke.poke.filter_rate;
			shoot.poke_current[0]=pid_double_loop_cal(&pid_trigger_angle[0],&pid_trigger_speed[0],
																								  shoot.poke_pid.angle_ref[0],
																								  shoot.poke_pid.angle_fdb[0],
																								 &shoot.poke_pid.speed_ref[0],
																									shoot.poke_pid.speed_fdb[0],0);
			}
			if((shoot.fric_wheel_run)&&(lock_rotor1 == 0))

			{	
/**/
			shoot.poke_pid.angle_ref[0]=shoot.poke_pid.angle_fdb[0]+shoot.shoot_frequency*45*36/500;		//一秒shoot_frequency发，一发拨盘转45°，减速比是1：36。每两毫秒执行一次故除以500。	
			shoot.poke_pid.angle_fdb[0]=general_poke.poke.ecd_angle;
			shoot.poke_pid.speed_fdb[0]=general_poke.poke.filter_rate;

			shoot.poke_current[0]=pid_double_loop_cal(&pid_trigger_angle[0],&pid_trigger_speed[0],
																								  shoot.poke_pid.angle_ref[0],
																								  shoot.poke_pid.angle_fdb[0],
																								 &shoot.poke_pid.speed_ref[0],
																									shoot.poke_pid.speed_fdb[0],0);
//			shoot.poke_pid.speed_ref[0]=-250;
//			shoot.poke_current[0]=pid_calc(&pid_trigger_speed[0],shoot.poke_pid.speed_fdb[0],shoot.poke_pid.speed_ref[0]);
			}
		}
		else
		{
		pid_trigger_angle[0].set = pid_trigger_angle[0].get;//松手必须停
		shoot.poke_current[0]=0;
		start_shooting_count = 0;//清零正转计时
    start_reversal_count1 = 0;//清零反转计时
		}		
}
	else//打幅单发模式
	{
//		if(buff_poke_init==0)		
//			{pid_trigger_angle[0].set = pid_trigger_angle[0].get;
//			shoot.poke_pid.angle_ref[0]=shoot.poke_pid.angle_fdb[0];
//			 buff_poke_init=1;}
			
		if(shoot.fric_wheel_run==1)
		{
			
			if(RC_CtrlData.mouse.press_l==1)
			 {if(press_l_flag==0)
				{
				press_l_flag=1;	
				shoot.poke_run=1;	
				}
				else
				{
				shoot.poke_run=0;	
				}
			}
		else
				{
					press_l_flag=0;			
				}	
/**/			
//		if(rotate_flag==0)
//				{			
/**/				
					if(shoot.poke_run==1)
					{
						shoot.poke_pid.angle_ref[0]+=shoot.single_angle;
//						reverse_once_flag=1;
					}
						

						shoot.poke_pid.angle_fdb[0]=general_poke.poke.ecd_angle/36.109f;
						shoot.poke_pid.speed_fdb[0]=general_poke.poke.rate_rpm;
						shoot.poke_current[0]=pid_double_loop_cal(&pid_trigger_angle_buf,&pid_trigger_speed_buf,
																		shoot.poke_pid.angle_ref[0],
																		shoot.poke_pid.angle_fdb[0],
																		&shoot.poke_pid.speed_ref[0],
																		shoot.poke_pid.speed_fdb[0],0);
/**/					
//				if(fabs(shoot.poke_pid.angle_ref[0]-shoot.poke_pid.angle_fdb[0])<0.5f && reverse_once_flag==1)
//					{
//					poke_cnt++;
//					if(poke_cnt>20)
//					 {	
//						poke_reach_flag=1;
//						rotate_flag=1;
//					 }
//				  }
//				}
//				else//开始反转
//				{
//						if(poke_reach_flag==1)
//					{
//					 poke_cnt=0;
//					 shoot.poke_pid.speed_ref[0]=-400;
//					 shoot.poke_pid.speed_fdb[0]=general_poke.poke.rate_rpm;
//					 shoot.poke_current[0]=pid_calc(&pid_trigger_speed[0],shoot.poke_pid.speed_fdb[0],shoot.poke_pid.speed_ref[0]);
//					}
//					
//					if(fabs(shoot.poke_pid.speed_fdb[0]<5))
//						 {
//							 poke_lock_cnt++;
//								if(poke_lock_cnt>200)
//								{
//									
//									poke_reach_flag=0;
//									poke_lock_flag=1;
//									poke_lock_cnt=0;
//								}
//							}
//							
//					if(poke_lock_flag==1)
//					{
//						shoot.poke_pid.angle_fdb[0]=general_poke.poke.ecd_angle/36.109f;
//            shoot.poke_pid.angle_ref[0]=shoot.poke_pid.angle_fdb[0];
//						buff_poke_init=0;
//						poke_lock_flag=0;
//						reverse_once_flag=0;
//						rotate_flag=0;
//					}
//				}
/**/
		}
		else
		{
			pid_trigger_angle[0].set = pid_trigger_angle[0].get;
      shoot.poke_pid.angle_ref[0]=shoot.poke_pid.angle_fdb[0];
			shoot.poke_current[0]=0;
    }
	}
}

void shoot_bullet_handle1(void)
{	

	shoot.single_angle=45;

	heat_shoot_frequency_limit();//步兵射频限制

	if(gimbal_data.ctrl_mode!=GIMBAL_AUTO_SMALL_BUFF&&
		 gimbal_data.ctrl_mode!=GIMBAL_AUTO_BIG_BUFF)//正常模式   
	  {//热量限制
		  if(shoot.will_time_shoot>0&&
				 shoot.fric_wheel_run==1&&
							 shoot.poke_run==1&&				
						  shoot.ctrl_mode==1)
//			  if(shoot.fric_wheel_run==1&&
//							 shoot.poke_run==1)	
		{	
/**/
//			start_shooting_count++;
//			if((start_shooting_count >= 250)&&(abs(general_poke.poke.filter_rate) < 3))
//			{
//				lock_rotor1 = 1;
//				start_shooting_count = 0;
//			}
//			if(lock_rotor1 == 1)
//			{
//				start_reversal_count1++;
//			if(start_reversal_count1 > 150)
//			{
//				lock_rotor1 = 0;
//				start_reversal_count1 = 0;
//			}
//			shoot.poke_pid.angle_ref[0]=shoot.poke_pid.angle_fdb[0]+shoot.shoot_frequency*45*36/500;		//一秒shoot_frequency发，一发拨盘转45°，减速比是1：36。每两毫秒执行一次故除以500。	
//			shoot.poke_pid.angle_fdb[0]=general_poke.poke.ecd_angle;
//			shoot.poke_pid.speed_fdb[0]=general_poke.poke.filter_rate;
//			shoot.poke_current[0]=pid_double_loop_cal(&pid_trigger_angle[0],&pid_trigger_speed[0],
//																								  shoot.poke_pid.angle_ref[0],
//																								  shoot.poke_pid.angle_fdb[0],
//																								 &shoot.poke_pid.speed_ref[0],
//																									shoot.poke_pid.speed_fdb[0],0);
//			}
//			if((shoot.fric_wheel_run)&&(lock_rotor1 == 0))

//			{	
/**/
			shoot.poke_pid.angle_ref[0]=shoot.poke_pid.angle_fdb[0]+shoot.shoot_frequency*45*36/500;		//一秒shoot_frequency发，一发拨盘转45°，减速比是1：36。每两毫秒执行一次故除以500。	
			shoot.poke_pid.angle_fdb[0]=general_poke.poke.ecd_angle;
			shoot.poke_pid.speed_fdb[0]=general_poke.poke.filter_rate;

			shoot.poke_current[0]=pid_double_loop_cal(&pid_trigger_angle[0],&pid_trigger_speed[0],
																								  shoot.poke_pid.angle_ref[0],
																								  shoot.poke_pid.angle_fdb[0],
																								 &shoot.poke_pid.speed_ref[0],
																									shoot.poke_pid.speed_fdb[0],0);
//			shoot.poke_pid.speed_ref[0]=-250;
//			shoot.poke_current[0]=pid_calc(&pid_trigger_speed[0],shoot.poke_pid.speed_fdb[0],shoot.poke_pid.speed_ref[0]);
//			}
		}
		else
		{
		pid_trigger_angle[0].set = pid_trigger_angle[0].get;//松手必须停
		shoot.poke_current[0]=0;
		start_shooting_count = 0;//清零正转计时
    start_reversal_count1 = 0;//清零反转计时
		}		
}
	else//打幅单发模式
	{
//		if(buff_poke_init==0)		
//			{pid_trigger_angle[0].set = pid_trigger_angle[0].get;
//			shoot.poke_pid.angle_ref[0]=shoot.poke_pid.angle_fdb[0];
//			 buff_poke_init=1;}
			
		if(shoot.fric_wheel_run==1)
		{
			
			if(RC_CtrlData.mouse.press_l==1)
			 {if(press_l_flag==0)
				{
				press_l_flag=1;	
				shoot.poke_run=1;	
				}
				else
				{
				shoot.poke_run=0;	
				}
			}
		else
				{
					press_l_flag=0;			
				}	
/**/			
//		if(rotate_flag==0)
//				{			
/**/				
					if(shoot.poke_run==1)
					{
						shoot.poke_pid.angle_ref[0]+=shoot.single_angle;
//						reverse_once_flag=1;
					}
						

						shoot.poke_pid.angle_fdb[0]=general_poke.poke.ecd_angle/36.109f;
						shoot.poke_pid.speed_fdb[0]=general_poke.poke.rate_rpm;
						shoot.poke_current[0]=pid_double_loop_cal(&pid_trigger_angle_buf,&pid_trigger_speed_buf,
																		shoot.poke_pid.angle_ref[0],
																		shoot.poke_pid.angle_fdb[0],
																		&shoot.poke_pid.speed_ref[0],
																		shoot.poke_pid.speed_fdb[0],0);
/**/					
//				if(fabs(shoot.poke_pid.angle_ref[0]-shoot.poke_pid.angle_fdb[0])<0.5f && reverse_once_flag==1)
//					{
//					poke_cnt++;
//					if(poke_cnt>20)
//					 {	
//						poke_reach_flag=1;
//						rotate_flag=1;
//					 }
//				  }
//				}
//				else//开始反转
//				{
//					if(poke_reach_flag==1)
//					{
//					 poke_cnt=0;
//					 shoot.poke_pid.speed_ref[0]=-400;
//					 shoot.poke_pid.speed_fdb[0]=general_poke.poke.rate_rpm;
//					 shoot.poke_current[0]=pid_calc(&pid_trigger_speed[0],shoot.poke_pid.speed_fdb[0],shoot.poke_pid.speed_ref[0]);
//					}
//					
//					if(fabs(shoot.poke_pid.speed_fdb[0]<5))/*堵转结束条件*/
//						 {
//							 poke_lock_cnt++;
//								if(poke_lock_cnt>200)
//								{
//									
//									poke_reach_flag=0;
//									poke_lock_flag=1;
//									poke_lock_cnt=0;
//								}
//							}
//							
//					if(poke_lock_flag==1)
//					{
//						shoot.poke_pid.angle_fdb[0]=general_poke.poke.ecd_angle/36.109f;
//            shoot.poke_pid.angle_ref[0]=shoot.poke_pid.angle_fdb[0];
//						buff_poke_init=0;
//						poke_lock_flag=0;
//						reverse_once_flag=0;
//						rotate_flag=0;
//					}
//				}
/**/
		}
		else
		{
			pid_trigger_angle[0].set = pid_trigger_angle[0].get;
      shoot.poke_pid.angle_ref[0]=shoot.poke_pid.angle_fdb[0];
			shoot.poke_current[0]=0;
    }
	}
}
#endif

                                

#if SHOOT_TYPE == 3//摩擦轮 friction
void shoot_friction_handle()
{  
	if(shoot.fric_wheel_run==1)
	{
		pid_rotate[0].set=-shoot.friction_pid.speed_ref[0] + Shooter_Bullet_Speed_Self_Adaptation(shoot.Bullet_Speed_Kalman.X_hat);
		pid_rotate[1].set= shoot.friction_pid.speed_ref[0] - Shooter_Bullet_Speed_Self_Adaptation(shoot.Bullet_Speed_Kalman.X_hat);	
	}
	else
	{
    pid_rotate[0].set=0;
    pid_rotate[1].set=0;
  }
	pid_rotate[0].get = general_friction.left_motor.filter_rate;
  pid_rotate[1].get = general_friction.right_motor.filter_rate;

  shoot.fric_current[0]=pid_calc(& pid_rotate[0],pid_rotate[0].get, pid_rotate[0].set);
  shoot.fric_current[1]=pid_calc(& pid_rotate[1],pid_rotate[1].get, pid_rotate[1].set);
}	


#endif

/**
************************************************************************************************************************
* @Name     : shoot_state_mode_switch
* @brief    : 遥控器/键鼠 发射状态更新
* @retval   : 
* @Note     : 
************************************************************************************************************************
**/	uint16_t friction_cnt=0;
void shoot_state_mode_switch()
{

	 /****************************键鼠射击状态更新**********************************************/		
		switch(RC_CtrlData.inputmode)
			{
					case REMOTE_INPUT:
				{
					if(RC_CtrlData.RemoteSwitch.s3to1)
					{
						shoot.fric_wheel_run=1;
						LASER_ON();
					}
					 else
					{	
						shoot.fric_wheel_run=0;
						LASER_OFF();
					}
					if(RC_CtrlData.rc.ch4>1500)
							shoot.poke_run=1;
					 else
							shoot.poke_run=0;
				}break;
					case KEY_MOUSE_INPUT:
				{
					if(RC_CtrlData.mouse.press_l==1)
							shoot.poke_run=1;
					else
							shoot.poke_run=0;
					 
					if(RC_CtrlData.Key_Flag.Key_C_TFlag&&shoot.fric_wheel_run==0)
					{
						friction_cnt=0;
						LASER_ON();
						shoot.fric_wheel_run=1;
					}
					else if(RC_CtrlData.Key_Flag.Key_C_Flag&&shoot.fric_wheel_run==1)
					{
						friction_cnt++;
						if(friction_cnt>=1000)
						{
						 LASER_OFF();
						 shoot.fric_wheel_run=0;
						 
						}
					}
//					 
//					if(RC_CtrlData.Key_Flag.Key_Q_TFlag)
//						 {shoot.bulletspead_level=1;}
//					else
//						 {shoot.bulletspead_level=0;}
				 }break;

					default:
					{
						shoot.poke_run=0;
						shoot.fric_wheel_run=0;
					}
					break;		
			}
}

/**
************************************************************************************************************************
* @Name     : shoot_task
* @brief    : 发射函数主函数
* @retval   : 
* @Note     : 
************************************************************************************************************************
**/
void shoot_task()
{

	shoot_state_mode_switch();


	shoot_bullet_handle();
	shoot_friction_handle();

#if SHOOT_TYPE != 6//飞机没热量
	heat0_limit();
#endif
}

