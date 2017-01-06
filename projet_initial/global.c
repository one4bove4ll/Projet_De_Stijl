/*
 * File:   global.h
 * Author: pehladik
 *
 * Created on 21 avril 2011, 12:14
 */

#include "global.h"

RT_TASK tServeur;
RT_TASK tconnect;
RT_TASK tmove;
RT_TASK tenvoyer;
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
RT_MUTEX mutexPosition ;


RT_SEM semConnecterRobot;

RT_QUEUE queueMsgGUI;

int etatCommWebcam ;
int etatCommMoniteur = 1;
int etatCommRobot = 1;
DRobot *robot;
DMovement *move;
DServer *serveur;
DCamera* webcam ;
DArena* arena ;
DPosition* position ;



int MSG_QUEUE_SIZE = 10;

int PRIORITY_TSERVEUR = 30;
int PRIORITY_TCONNECT = 20;
int PRIORITY_TMOVE = 10;
int PRIORITY_TENVOYER = 25;
int PRIORITY_TBATTERIE = 35;
