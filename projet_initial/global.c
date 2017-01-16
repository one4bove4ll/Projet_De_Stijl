/*
 * File:   global.h
 * Author: pehladik
 *
 * Created on 21 avril 2011, 12:14
 */

#include "global.h"

RT_TASK tarena ;
RT_TASK tServeur;
RT_TASK tconnect;
RT_TASK tmove;
RT_TASK tenvoyer;
RT_TASK tcomm; /* task liée à la fonction "void wtd_ctrl_comm_sup_rob(void * arg)" */
RT_TASK tbatterie;
RT_TASK timage;


RT_MUTEX mutexEtatCamera ;
RT_MUTEX mutexEtatCommMoniteur ;
RT_MUTEX mutexEtat;
RT_MUTEX mutexRobot ;
RT_MUTEX mutexMove;
RT_MUTEX mutexServeur ;
RT_MUTEX mutexCamera ;
RT_MUTEX mutexArena ;
RT_MUTEX mutexComputePosition ;
RT_MUTEX mutexCptErrors ;
RT_MUTEX mutexEtatThArena ;

RT_SEM semConnecterRobot;
RT_SEM semArena ;

RT_QUEUE queueMsgGUI;

int etatComputePosition = 0;
int etatCommWebcam ;
int etatCommMoniteur = 1;
int etatCommRobot = 1;
int compteur_errors = 0;
int etatThArena =0 ;
DRobot *robot;
DMovement *move;
DServer *serveur;
DCamera* webcam ;
DArena* arena ;

int MSG_QUEUE_SIZE = 10;

int PRIORITY_TCONNECT = 20;
int PRIORITY_TMOVE = 10;
int PRIORITY_TENVOYER = 55;
int PRIORITY_TBATTERIE = 5;
int PRIORITY_TIMAGE = 19;
int PRIORITY_TARENA = 24;
int PRIORITY_TSERVEUR = 25;
int PRIORITY_TCOMM = 50;
