#ifndef __CANBUS_H
#define __CANBUS_H
#include "public.h"


/**********************舵轮底盘电机id||麦轮底盘电机id**************************************/
#define GM1Encoder_MOTOR 0x00
#define GM2Encoder_MOTOR 0X00
#define GM3Encoder_MOTOR 0X00
#define GM4Encoder_MOTOR 0X00

#define CM1Encoder_MOTOR 0x00
#define CM2Encoder_MOTOR 0x00
#define CM3Encoder_MOTOR 0x00
#define CM4Encoder_MOTOR 0x00
/*************************云台电机id******************************/
#define GIMBAL_YAW_MOTOR 1
#define GIMBAL_PITCH_MOTOR 0X206
/****************************英雄小云台电机id***********************************/
#define SMALL_GIMBAL_MOTOR 0X00
#define SCOPE_MOTOR 0X00
/*********************************摩擦轮电机id**************************************/
#define LEFT_FRICTION 0X203
#define RIGHT_FRICTION 0X201
//哨兵
#define LEFT_UP_FRICTION 0X00
#define RIGHT_UP_FRICTION 0X00
#define LEFT_DOWN_FRIICTION 0X00
#define RIGHT_DOWN_FRICTION 0X00
/**********************************拨盘电机id**************************************/
#define DOWN_POKE 0x205    
#define UP_POKE 0X205
#define LEFT_POKE 0X00			//右一左二，只有一个用一
#define RIGHT_POKE 0X00
#define POKE 0X202

#define BULLET_HATCH 0x204
/*********************************舵轮上下板通信id*********************************/
#define UP_CAN2_TO_DOWN_CAN1_1 0X407
#define UP_CAN2_TO_DOWN_CAN1_2 0X408
#define UP_CAN2_TO_DOWN_CAN1_3 0X409



void Can1ReceiveMsgProcess(CanRxMsg * msg);
void Can2ReceiveMsgProcess(CanRxMsg * msg);




										
void can_bus_send_task(void);

#endif
