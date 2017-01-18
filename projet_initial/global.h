/* 
 * File:   global.h
 * Author: pehladik
 *
 * Created on 12 janvier 2012, 10:11
 */

#ifndef GLOBAL_H
#define	GLOBAL_H

#include "includes.h"

/* @descripteurs des tâches */
extern RT_TASK tarena;
extern RT_TASK tServeur;
extern RT_TASK tconnect;
extern RT_TASK tmove;
extern RT_TASK tenvoyer;
extern RT_TASK tcomm;
extern RT_TASK tbatterie ;
extern RT_TASK timage ;

/* @descripteurs des mutex */
extern RT_MUTEX mutexEtat;
extern RT_MUTEX mutexMove;
extern RT_MUTEX mutexRobot ;
extern RT_MUTEX mutexEtatCommMoniteur ;
extern RT_MUTEX mutexEtatCamera ;
extern RT_MUTEX mutexCamera ;
extern RT_MUTEX mutexArena ;
extern RT_MUTEX mutexComputePosition ;
extern RT_MUTEX mutexServeur ;
extern RT_MUTEX mutexCptErrors ;
extern RT_MUTEX mutexEtatThArena ;
/* @descripteurs des sempahore */
extern RT_SEM semConnecterRobot;
extern RT_SEM semArena ;

/* @descripteurs des files de messages */
extern RT_QUEUE queueMsgGUI;

/* @variables partagées */
extern int etatComputePosition;
extern int etatCommMoniteur;
extern int etatCommRobot;
extern int compteur_errors ;
extern int etatThArena ;
extern DServer *serveur;
extern DRobot *robot;
extern DMovement *move;
extern DCamera* webcam ;
extern DArena* arena ;
extern DPosition* position;
/* @constantes */



extern int MSG_QUEUE_SIZE;

extern int PRIORITY_TSERVEUR;
extern int PRIORITY_TCONNECT;
extern int PRIORITY_TMOVE;
extern int PRIORITY_TENVOYER;
extern int PRIORITY_TCOMM; /* priorité de la tâche wdt_ctrl_comm_sup_rob */
extern int PRIORITY_TBATTERIE;
extern int PRIORITY_TIMAGE ;
extern int PRIORITY_TARENA ;

#endif	/* GLOBAL_H */

