/*
 * File:   global.h
 * Author: pehladik
 *
 * Created on 21 avril 2011, 12:14
 */

#include "global.h"

<<<<<<< HEAD
=======
RT_TASK tarena ;
>>>>>>> 449c872fe420a7a32aaaeadea45a3a270bf9f1f7
RT_TASK tServeur;
RT_TASK tconnect;
RT_TASK tmove;
RT_TASK tenvoyer;
<<<<<<< HEAD
RT_TASK tcomm; /* task liée à la fonction "void wtd_ctrl_comm_sup_rob(void * arg)" */

RT_MUTEX mutexEtat;
RT_MUTEX mutexMove;
RT_MUTEX mutexRobot;
RT_MUTEX mutexCptErrors;
RT_MUTEX mutexServeur;

RT_SEM semConnecterRobot;

RT_QUEUE queueMsgGUI;

=======
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
RT_MUTEX mutexCptErrors ;
RT_MUTEX mutexEtatThArena ;

RT_SEM semConnecterRobot;
RT_SEM semArena ;

RT_QUEUE queueMsgGUI;

int etatCommWebcam ;
>>>>>>> 449c872fe420a7a32aaaeadea45a3a270bf9f1f7
int etatCommMoniteur = 1;
int etatCommRobot = 1;
DRobot *robot;
DMovement *move;
DServer *serveur;
<<<<<<< HEAD

int compteur_errors = 0;


int MSG_QUEUE_SIZE = 10;

int PRIORITY_TSERVEUR = 22;
int PRIORITY_TCONNECT = 20;
int PRIORITY_TMOVE = 10;
int PRIORITY_TENVOYER = 25;
int PRIORITY_TCOMM = 35; /* voir si cela est bien */
=======
DCamera* webcam ;
DArena* arena ;
DPosition* position ;
int compteur_errors ;
int etatThArena =0 ;

int MSG_QUEUE_SIZE = 10;

int PRIORITY_TSERVEUR = 30;
int PRIORITY_TCONNECT = 20;
int PRIORITY_TMOVE = 10;
int PRIORITY_TENVOYER = 25;
int PRIORITY_TBATTERIE = 35;
int PRIORITY_TIMAGE = 40;
int PRIORITY_TARENA = 45;
>>>>>>> 449c872fe420a7a32aaaeadea45a3a270bf9f1f7
