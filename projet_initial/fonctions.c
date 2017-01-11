#include "fonctions.h"

int write_in_queue(RT_QUEUE *msgQueue, void * data, int size);

void envoyer(void * arg) {
  DMessage *msg;
  int err;

  while (1) {
    rt_printf("tenvoyer : Attente d'un message\n");
    if ((err = rt_queue_read(&queueMsgGUI, &msg, sizeof (DMessage), TM_INFINITE)) >= 0) {
      rt_printf("tenvoyer : envoi d'un message au moniteur\n");
      serveur->send(serveur, msg);
      msg->free(msg);
    } else {
      rt_printf("Error msg queue write: %s\n", strerror(-err));
    }
  }
}

void connecter(void * arg) {
  int status;
  DMessage *message;

  rt_printf("tconnect : Debut de l'exécution de tconnect\n");

  while (1) {
    rt_printf("tconnect : Attente du sémarphore semConnecterRobot\n");
    rt_sem_p(&semConnecterRobot, TM_INFINITE);
    rt_printf("tconnect : Ouverture de la communication avec le robot\n");
    status = robot->open_device(robot);

    rt_mutex_acquire(&mutexEtat, TM_INFINITE);
    etatCommRobot = status;
    rt_mutex_release(&mutexEtat);

    if (status == STATUS_OK) {
      status = robot->start_insecurely(robot);
      if (status == STATUS_OK){
	rt_printf("tconnect : Robot démarrer\n");
      }
    }

    message = d_new_message();
    message->put_state(message, status);

    rt_printf("tconnecter : Envoi message\n");
    message->print(message, 100);

    if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
      message->free(message);
    }
  }
}

void communiquer(void *arg) {
  DMessage *msg = d_new_message();
  int var1 = 1;
  int num_msg = 0;

  rt_printf("tserver : Début de l'exécution de serveur\n");
  serveur->open(serveur, "8000");
  rt_printf("tserver : Connexion\n");

  rt_mutex_acquire(&mutexEtat, TM_INFINITE);
  etatCommMoniteur = 0;
  rt_mutex_release(&mutexEtat);

  while (var1 > 0) {
    rt_printf("tserver : Attente d'un message\n");
    var1 = serveur->receive(serveur, msg);
    num_msg++;
    if (var1 > 0) {
      switch (msg->get_type(msg)) {
      case MESSAGE_TYPE_ACTION:
	rt_printf("tserver : Le message %d reçu est une action\n",
		  num_msg);
	DAction *action = d_new_action();
	action->from_message(action, msg);
	switch (action->get_order(action)) {
	case ACTION_CONNECT_ROBOT:
	  rt_printf("tserver : Action connecter robot\n");
	  rt_sem_v(&semConnecterRobot);
	  break;
	case ACTION_FIND_ARENA:
	  rt_printf("tserver : Demande de calibration de l'arene\n");
	  rt_mutex_acquire(&mutexEtatThArena,TM_INFINITE);
	  etatThArena = 0 ;
	  rt_mutex_release(&mutexEtatThArena);
	  rt_sem_v(&semArena);
	  break;
	case ACTION_ARENA_IS_FOUND:
	  rt_printf("tserver : Arene Trouvée !! \n");
	  rt_mutex_acquire(&mutexEtatThArena,TM_INFINITE);
	  etatThArena = 1 ;
	  rt_mutex_release(&mutexEtatThArena);
	  rt_sem_v(&semArena);
	  break;
	case ACTION_ARENA_FAILED:
	  rt_printf("tserver : Arene non trouvée :( \n");
	  rt_mutex_acquire(&mutexEtatThArena,TM_INFINITE);
	  etatThArena = -1 ;
	  rt_mutex_release(&mutexEtatThArena);
	  rt_sem_v(&semArena);
	  break;
	}
	break;
      case MESSAGE_TYPE_MOVEMENT:
	rt_printf("tserver : Le message reçu %d est un mouvement\n",
		  num_msg);
	rt_mutex_acquire(&mutexMove, TM_INFINITE);
	move->from_message(move, msg);
	move->print(move);
	rt_mutex_release(&mutexMove);
	break;
      }
    }
  }
}

void deplacer(void *arg) {
  int status = 1;
  int gauche;
  int droite;
  DMessage *message;

  rt_printf("tmove : Debut de l'éxecution de periodique à 1s\n");
  rt_task_set_periodic(NULL, TM_NOW, 500000000);

  while (1) {
    /* Attente de l'activation périodique */
    rt_task_wait_period(NULL);
    rt_printf("tmove : Activation périodique\n");

    rt_mutex_acquire(&mutexEtat, TM_INFINITE);
    status = etatCommRobot;
    rt_mutex_release(&mutexEtat);

    if (status == STATUS_OK) {
      rt_mutex_acquire(&mutexMove, TM_INFINITE);
      switch (move->get_direction(move)) {
      case DIRECTION_FORWARD:
	gauche = MOTEUR_ARRIERE_LENT;
	droite = MOTEUR_ARRIERE_LENT;
	break;
      case DIRECTION_LEFT:
	gauche = MOTEUR_ARRIERE_LENT;
	droite = MOTEUR_AVANT_LENT;
	break;
      case DIRECTION_RIGHT:
	gauche = MOTEUR_AVANT_LENT;
	droite = MOTEUR_ARRIERE_LENT;
	break;
      case DIRECTION_STOP:
	gauche = MOTEUR_STOP;
	droite = MOTEUR_STOP;
	break;
      case DIRECTION_STRAIGHT:
	gauche = MOTEUR_AVANT_LENT;
	droite = MOTEUR_AVANT_LENT;
	break;
      }
      rt_mutex_release(&mutexMove);

      status = robot->set_motors(robot, gauche, droite);

      if (status != STATUS_OK) {
	rt_mutex_acquire(&mutexEtat, TM_INFINITE);
	etatCommRobot = status;
	rt_mutex_release(&mutexEtat);

	message = d_new_message();
	message->put_state(message, status);

	rt_printf("tmove : Envoi message\n");
	if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
	  message->free(message);
	}
      }
    }
  }
}

void th_battery(void * arg){
  
  DBattery* bat = d_new_battery();
  DMessage* msg ;
  int valbat ;
  int status_robot = 1 ;
  int status_moniteur =1 ;
  //description du caractere periodique du thread (toutes les 250ms)

  rt_printf("tbatterie : Debut de l'éxecution de periodique à 250ms\n");
  rt_task_set_periodic(NULL, TM_NOW, 250000000);

  while(1){
    rt_task_wait_period(NULL);
    rt_printf("tbatterie : Activation périodique\n");

    //je verifie la connexion avec le robot
    rt_mutex_acquire(&mutexEtat, TM_INFINITE);
    status_robot = etatCommRobot;
    //rt_printf("\n\ntbatterie : status lu : %d\n\n\n", status); //debug
    rt_mutex_release(&mutexEtat);

    //je verifie la connexion avec le moniteur
    rt_mutex_acquire(&mutexEtatCommMoniteur, TM_INFINITE);
    status_moniteur = etatCommMoniteur ;
    rt_mutex_release(&mutexEtatCommMoniteur);


    if (status_robot == STATUS_OK && status_moniteur == STATUS_OK) {
      //zone critique pour le robot
      rt_mutex_acquire(&mutexRobot, TM_INFINITE);
      //je récupère la valeur de la batterie
      d_robot_get_vbat(robot,&valbat);
      rt_mutex_release(&mutexRobot);

      if(valbat ==-1){//erreur de connexion
	rt_mutex_acquire(&mutexCptErrors,TM_INFINITE);
	compteur_errors++;
	rt_mutex_release(&mutexCptErrors);
       }else {//connexion ok
	rt_mutex_acquire(&mutexCptErrors,TM_INFINITE);
	compteur_errors=0;
	rt_mutex_release(&mutexCptErrors);
        //j'envoie le Dmessage au moniteur
        d_battery_set_level(bat,valbat);
        msg = d_new_message(); 
        d_message_put_battery_level(msg,bat);
        write_in_queue(&queueMsgGUI,msg,sizeof(DMessage));
        //rt_printf("sizeof(msg) = %d\n",sizeof(DMessage));//debug
      }
    }
    
  }

  d_message_free(msg);
}

void image(void * arg) {
	
	DImage* img;
	DMessage* msg;
	DJpegimage* imgjpg ;
//	DCamera* webcam ;

	rt_printf("timage : Debut de l'éxecution de periodique à 600ms\n");
	rt_task_set_periodic(NULL, TM_NOW, 600000000);
	while(1){
		rt_task_wait_period(NULL);
		rt_printf("timage : Activation périodique\n");
		rt_mutex_acquire(&mutexCamera,TM_INFINITE); //debut de la section critique a ajuster !
		/*initialisation des variables*/
//		webcam = d_new_camera();
		img = d_new_image();
		msg = d_new_message();			
		imgjpg = d_new_jpegimage();
		/*----------------------------*/

		/*--acquisition de l'image---*/
		//d_camera_open(webcam);
		d_camera_get_frame(webcam,img);
		//d_camera_close(webcam);
		/*--------------------------*/

		rt_mutex_acquire(&mutexEtatThArena,TM_INFINITE);
		if(etatThArena == 1){
			rt_mutex_acquire(&mutexArena,TM_INFINITE);
			d_imageshop_draw_arena(img,arena);
			rt_mutex_release(&mutexArena);
		}
		rt_mutex_release(&mutexEtatThArena);
		

		d_jpegimage_compress(imgjpg,img);  //compression
		d_message_put_jpeg_image(msg,imgjpg); //creation message
		write_in_queue(&queueMsgGUI,msg,sizeof(DMessage)); //envoi message 	

//		d_message_free(msg);
//		d_image_free(img);
//		d_jpegimage_free(imgjpg);
		rt_mutex_release(&mutexCamera); //fin section critique
	}

}


void th_arena(void * arg){
  int status ;	
  DMessage *msg;
  DImage *img ;
  DJpegimage* imgjpg ;

  rt_printf("tconnect : Debut de l'exécution de tarena\n");

  while (1) {
    rt_printf("tarena : Attente du sémaphore semArena\n");
    rt_sem_p(&semArena, TM_INFINITE);
    rt_printf("tarena : Début de la calibration de l'arene\n");   
 
    /*----------PARTIE 1 Calcul et detection de l'arene------------*/
    /*------------Verification Communication Moniteur--------------*/
    rt_mutex_acquire(&mutexEtatCommMoniteur, TM_INFINITE);
    etatCommMoniteur = status;
    rt_mutex_release(&mutexEtatCommMoniteur);

    if(status == STATUS_OK){
	rt_mutex_acquire(&mutexCamera, TM_INFINITE);
    /*----------------Initialisation des variables-----------------*/
	img = d_new_image();
	msg = d_new_message();
	imgjpg= d_new_jpegimage();
    /*-------------------------------------------------------------*/

    /*------------------Acquisition de l'image---------------------*/
	d_camera_get_frame(webcam,img);
    /*-------------------------------------------------------------*/
    /*----------------Detection et trace de l'arene----------------*/
	rt_mutex_acquire(&mutexArena,TM_INFINITE);

	arena = d_image_compute_arena_position(img);
	d_imageshop_draw_arena(img,arena);

	rt_mutex_release(&mutexArena);
    /*-------------------------------------------------------------*/
    /*--------------Compression et envoi de l'image----------------*/
	d_jpegimage_compress(imgjpg,img);  //compression
	d_message_put_jpeg_image(msg,imgjpg); //creation message
	write_in_queue(&queueMsgGUI,msg,sizeof(DMessage)); //envoi message 
	rt_printf("Envoie de l'image au moniteur \n");
    /*-------------------------------------------------------------*/
    /*-------------PARTIE 2 Réponse de l'utilisateur---------------*/
	rt_printf("Attente de la réponse de l'utilisateur \n");
	rt_sem_p(&semArena, TM_INFINITE);
	rt_mutex_acquire(&mutexEtatThArena,TM_INFINITE);
	if(etatThArena == 1){
    /*--------------------------Arena OK--------------------------*/
		rt_printf("L'arene est bien enregistrée \n");
	}else if(etatThArena == -1) {
    /*------------------------Arena NON OK------------------------*/
		rt_printf("L'arene est jetee \n");
	}	
	rt_mutex_release(&mutexEtatThArena);

	rt_mutex_release(&mutexCamera);	
    }else {
	rt_printf("Connexion perdue avec le moniteur \n");
    }
  }

}

int write_in_queue(RT_QUEUE *msgQueue, void * data, int size) {
  void *msg;
  int err;

  msg = rt_queue_alloc(msgQueue, size);
  memcpy(msg, &data, size);

  if ((err = rt_queue_send(msgQueue, msg, sizeof (DMessage), Q_NORMAL)) < 0) {
    rt_printf("Error msg queue send: %s\n", strerror(-err));
  }
  rt_queue_free(&queueMsgGUI, msg);

  return err;
}
