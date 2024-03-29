#include "app.h"
#include "DD_Gene.h"
#include "DD_RCDefinition.h"
#include "SystemTaskManager.h"
#include <stdlib.h>
#include <stdbool.h>
#include "MW_GPIO.h"
#include "MW_IWDG.h"
#include "message.h"
#include "MW_flash.h"
#include "constManager.h"
#include "trapezoid_ctrl.h"

static
int steering_spin_to_target(int target,int target_motor);
static
int steering_1_init(void);
static
int steering_2_init(void);
static
int get_diff(int target,int now_degree,int encoder_ppr);
/*suspensionSystem*/
static
int suspensionSystem(void);
/*ABSystem*/
static 
int ABSystem(void);
static
int LEDSystem(void);
/*メモ
 *g_ab_h...ABのハンドラ
 *g_md_h...MDのハンドラ
 *
 *g_rc_data...RCのデータ
 */

int appInit(void){

  ad_init();

  /*GPIO の設定などでMW,GPIOではHALを叩く*/
  return EXIT_SUCCESS;
}

/*application tasks*/
int appTask(void){
  int ret=0;
  int target;

  static bool encoder1_reset = false;
  static bool encoder2_reset = false;
  
  if(__RC_ISPRESSED_R1(g_rc_data)&&__RC_ISPRESSED_R2(g_rc_data)&&
     __RC_ISPRESSED_L1(g_rc_data)&&__RC_ISPRESSED_L2(g_rc_data)){
    while(__RC_ISPRESSED_R1(g_rc_data)||__RC_ISPRESSED_R2(g_rc_data)||
	  __RC_ISPRESSED_L1(g_rc_data)||__RC_ISPRESSED_L2(g_rc_data))
        SY_wait(10);
    ad_main();
  }

  if(DD_RCGetRX(g_rc_data)<0){
    target = (int)(-(atan((double)DD_RCGetRY(g_rc_data)/(double)DD_RCGetRX(g_rc_data))*(180.0/3.141592)));
    if(target < 0){
      target = -target + 270;
    }else{
      target = 90-target + 180;
    }
    target = 360 - target;
  }else{
    target = (int)((atan((double)DD_RCGetRY(g_rc_data)/(double)DD_RCGetRX(g_rc_data))*(180.0/3.141592)));
    target += 90;
    target = 360 - target;
  }
  /* if(!encoder1_reset){ */
  /*   if(steering_1_init()==0){ */
  /*     encoder1_reset = true; */
  /*   } */
  /* } */
  if(!encoder2_reset){
    if(steering_2_init()==0){
      encoder2_reset = true;
    }
  }
  if(!encoder1_reset){
    if(steering_1_init()==0){
      encoder1_reset = true;
    }
  }
  if(encoder1_reset && encoder2_reset){
    ret = steering_spin_to_target(target,0);
  }
  if(ret){
    return ret;
  }
  if( g_SY_system_counter % _MESSAGE_INTERVAL_MS < _INTERVAL_MS ){
    MW_printf("Encoder_target[%3d]\n",target);
    if(encoder1_reset){
      MW_printf("encoder1_reset[true ]\n");
    }else{
      MW_printf("encoder1_reset[false]\n");
    }
    if(encoder2_reset){
      MW_printf("encoder2_reset[true ]\n");
    }else{
      MW_printf("encoder2_reset[false]\n");
    }
  }
  
  /*それぞれの機構ごとに処理をする*/
  /*途中必ず定数回で終了すること。*/
  ret = suspensionSystem();
  if(ret){
    return ret;
  }

  ret = ABSystem();
  if(ret){
    return ret;
  }

  ret = LEDSystem();
  if(ret){
    return ret;
  }
  
  return EXIT_SUCCESS;
}

static
int steering_spin_to_target(int target,int target_motor){
  const int32_t encoder_ppr = (512)*4;
  const int div = (2048*4)/encoder_ppr;
  /* const int target_deg[9] = {2048,1024,512,256,128,64,32,26,21}; */
  /* const int target_duty[9]   = {8000,3000,1000,120,110,100,90,85,80}; */
  const int target_deg[2][9]  = {{2048/div,1024/div,512/div,256/div,128/div,64/div,32/div,26/div,21/div},
				 {2048/div,1024/div,512/div,256/div,128/div,64/div,32/div,26/div,21/div}};
  const int target_duty[2][9] = {{9000,5000,3000,1400,1300,1250,1200,1150,1000},
                                 {9000,5000,3000,1400,1300,1250,1200,1150,1000}};
  int32_t encoder;
  int32_t encoder_degree;
  int32_t target_degree;
  int spin_direction = 1;
  int diff_from_target;
  int duty;
  int i,j;
  int choose_motor[2] = {};

  if(target_motor == 0){
    choose_motor[0] = 0;
    choose_motor[1] = 2;
  }else if(target_motor == 1){
    choose_motor[0] = 0;
    choose_motor[1] = 1;
  }else if(target_motor == 2){
    choose_motor[0] = 1;
    choose_motor[1] = 2;
  }
  
  for(j=choose_motor[0];j<choose_motor[1];j++){
    switch(j){
    case 0:
      encoder = DD_encoder1Get_int32();
      break;
    case 1:
      encoder = DD_encoder2Get_int32();
      break;
    }
    encoder_degree = encoder % encoder_ppr;
    target_degree  = (int)(((double)encoder_ppr/360.0)*(double)target);
  
  
    if(encoder_degree < 0){
      encoder_degree = encoder_ppr - abs(encoder_degree);
    }
    diff_from_target = get_diff(target_degree,encoder_degree,encoder_ppr);
    for(i=0;i<9;i++){
      if(abs(diff_from_target)>=target_deg[j][i]){
	duty = target_duty[j][i];
	break;
      }
      if(i==8){
	duty = 0;
      }
    }
    if(diff_from_target > 0){
      g_md_h[j].duty = duty;
      if(duty==0){
	g_md_h[j].mode = D_MMOD_BRAKE;
      }else{
	g_md_h[j].mode = D_MMOD_FORWARD;
      }
    }else{
      g_md_h[j].duty = duty;
      if(duty==0){
	g_md_h[j].mode = D_MMOD_BRAKE;
      }else{
	g_md_h[j].mode = D_MMOD_BACKWARD;
      }
    }
    if( g_SY_system_counter % _MESSAGE_INTERVAL_MS < _INTERVAL_MS ){
      switch(j){
      case 0:
	MW_printf("E1_degree[%3d]\n",(int)((double)encoder_degree*(double)(360.0/(double)encoder_ppr)));
	break;
      case 1:
	MW_printf("E2_degree[%3d]\n",(int)((double)encoder_degree*(double)(360.0/(double)encoder_ppr)));
	break;
      }
    }
  }
}

static
int steering_1_init(void){
  if(_IS_PRESSED_ENCODER1_RESET()){
    g_md_h[0].duty = 0; 
    g_md_h[0].mode = D_MMOD_BRAKE;
    DD_encoder1reset();
    return 0;
  }else{
    g_md_h[0].duty = 900; 
    g_md_h[0].mode = D_MMOD_FORWARD;
  }
  return -1;
}

static
int steering_2_init(void){
  if(_IS_PRESSED_ENCODER2_RESET()){
    g_md_h[1].duty = 0; 
    g_md_h[1].mode = D_MMOD_BRAKE;
    DD_encoder2reset();
    return 0;
  }else{
    g_md_h[1].duty = 850; 
    g_md_h[1].mode = D_MMOD_FORWARD;
  }
  return -1;
}

static
int get_diff(int target,int now_degree,int encoder_ppr){
  int adjust = now_degree - encoder_ppr/2;
  int target_adjust = target - adjust;
  if(target_adjust>=encoder_ppr) target_adjust = target_adjust % encoder_ppr;
  if(target_adjust<0) target_adjust = encoder_ppr - abs(target_adjust);
  return target_adjust - encoder_ppr/2;
}

static int LEDSystem(void){
  static int color_num = 0;
  static bool c_up_flag = true;
  if(!__RC_ISPRESSED_UP(g_rc_data)){
    c_up_flag = true;
  }
  if(c_up_flag && __RC_ISPRESSED_UP(g_rc_data)){
    c_up_flag = false;
    g_ld_h[0].mode[4] = color_num;
    g_ld_h[0].mode[5] = color_num;
    g_ld_h[0].mode[6] = color_num;
    color_num++;
    if(color_num>=22){
      color_num = 0;
    }
    //g_led_mode = lmode_1;
  }
  if(__RC_ISPRESSED_DOWN(g_rc_data)){
    g_ld_h[0].mode[4] = D_LMOD_NONE;
    g_ld_h[0].mode[5] = D_LMOD_NONE;
    g_ld_h[0].mode[6] = D_LMOD_NONE;
    //g_led_mode = lmode_2;
  }
  
  return EXIT_SUCCESS;
}

static 
int ABSystem(void){

  /* g_ab_h[0].dat = 0x00; */
  /* if(__RC_ISPRESSED_CIRCLE(g_rc_data)){ */
  /*   g_ab_h[0].dat |= AB0; */
  /* } */
  /* if(__RC_ISPRESSED_CROSS(g_rc_data)){ */
  /*   g_ab_h[0].dat |= AB1; */
  /* } */

  return EXIT_SUCCESS;
}


/*プライベート 足回りシステム*/
static
int suspensionSystem(void){
  const int num_of_motor = 2;/*モータの個数*/
  int rc_analogdata;/*アナログデータ*/
  unsigned int idx;/*インデックス*/
  int i;

  /*for each motor*/
  for(i=0;i<num_of_motor;i++){
    /*それぞれの差分*/
    rc_analogdata = DD_RCGetLY(g_rc_data);
    switch(i){
    case 0:
      idx = MECHA1_MD1;
      break;
    case 1:
      idx = MECHA1_MD2;
      break;
    default:return EXIT_FAILURE;
    }
    
    /*これは中央か?*/
    if(abs(rc_analogdata)==0){
      g_md_h[idx].mode = D_MMOD_FREE;
      g_md_h[idx].duty = 0;
    }
    else{
      if(rc_analogdata > 0){
	/*前後の向き判定*/
	g_md_h[idx].mode = D_MMOD_FORWARD;
      }
      else{
	g_md_h[idx].mode = D_MMOD_BACKWARD;
      }
      /*絶対値を取りDutyに格納*/
      g_md_h[idx].duty = abs(rc_analogdata) * MD_GAIN;
    }
  }
  /* if(__RC_ISPRESSED_RIGHT(g_rc_data)){ */
  /*   g_md_h[0].mode = D_MMOD_FORWARD; */
  /*   //g_md_h[1].mode = D_MMOD_FORWARD; */
  /*   g_md_h[0].duty = 900; */
  /*   //g_md_h[1].duty = 80; */
  /* } */
  /* if(__RC_ISPRESSED_LEFT(g_rc_data)){ */
  /*   g_md_h[0].mode = D_MMOD_BACKWARD; */
  /*   //g_md_h[1].mode = D_MMOD_FORWARD; */
  /*   g_md_h[0].duty = 900; */
  /*   //g_md_h[1].duty = 80; */
  /* } */
  return EXIT_SUCCESS;
}
