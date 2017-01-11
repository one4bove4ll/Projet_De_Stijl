#include "fonctions.h"

int write_in_queue(RT_QUEUE *msgQueue, void * data, int size);

void envoyer(void * arg) {
  DMessage *msg  ;
  int err;

  while (1) {
    rt_printf("tenvoyer : Attente d'un message\n");
    if ((err = rt_queue_read(&queueMsgGUI, &msg, sizeof (DMessage), TM_INFINITE)) >= 0) {
      rt_printf("tenvoyer : envoi d'un message au moniteur\n");
	  
    //  rt_mutex_acquire(&mutexServeur,TM_INFINITE);
      serveur->send(serveur, msg);
	 // rt_mutex_release(&mutexServeur);
      
      msg->free(msg);
    }else {
      rt_printf("Error msg queue write: %s\n", strerror(-err));
    }
  }
}


void wtd_ctrl_comm_sup_rob(void * arg){
  DMessage *msg = d_new_message();
  rt_printf("tcomm : Debut de l'éxecution de periodique à 1 s\n");
  rt_task_set_periodic(NULL, TM_NOW, 1000000000);
  int cond = 1;
  int nb_errors;
  while (cond) {
    /* Attente de l'activation périodique */
    rt_task_wait_period(NULL);
    rt_printf("tcomm : Activation périodique\n");
	
    /* reload du watchdog */
    rt_mutex_acquire(&mutexRobot,TM_INFINITE);
    robot->reload_wdt(robot);
	rt_mutex_release(&mutexRobot);
 
   /* controle de communication entre le superviseur et le robot */
	rt_mutex_acquire(&mutexCptErrors, TM_INFINITE);
	nb_errors = compteur_errors;
	rt_mutex_release(&mutexCptErrors);
	printf("compteur_errors = %d\n",nb_errors);    
	if(nb_errors >= 4){ /* à partir de 4 pertes de messages entre le robot et le superviseur, on considere que la communication est perdue */
      int status=STATUS_ERR_NO_FILE;  // !=STATUS_OK
      // envoyer un message au moniteur pour lui dire que la communication est perdue
      msg->put_state(msg, status);
      rt_printf("tcomm : Envoi message au moniteur de la part du serveur car perte de communication entre le superviseur et le robot\n");
      msg->print(msg, 100);
		  
      if (write_in_queue(&queueMsgGUI, msg, sizeof (DMessage)) < 0) {
    	msg->free(msg);
      }
      // revenir dans l'état initial : stopper communications avec le robot, se mettre en attente d'une demande de connexion avec le robot
      rt_mutex_acquire(&mutexRobot,TM_INFINITE);
      robot->close_com(robot);
      rt_mutex_release(&mutexRobot);
      //rt_task_delete(&wtd_ctrl_comm_sup_rob); 
			
      rt_mutex_acquire(&mutexEtat, TM_INFINITE);
      etatCommRobot = status;
      rt_mutex_release(&mutexEtat);

	rt_mutex_acquire(&mutexCptErrors, TM_INFINITE);
	compteur_errors=0;
	nb_errors = compteur_errors;
	rt_mutex_release(&mutexCptErrors);
	cond = 0;
    }
  }
}


void connecter(void * arg) {
  int status, err;
  DMessage *message;

  rt_printf("tconnect : Debut de l'exécution de tconnect\n");

  while (1) {
    rt_printf("tconnect : Attente du sémarphore semConnecterRobot\n");
    rt_sem_p(&semConnecterRobot, TM_INFINITE);

    rt_printf("tconnect : Ouverture de la communication avec le robot\n");
	rt_mutex_acquire(&mutexRobot,TM_INFINITE);
    status = robot->open_device(robot);
    rt_mutex_release(&mutexRobot);

    rt_mutex_acquire(&mutexEtat, TM_INFINITE);
    etatCommRobot = status;
    rt_mutex_release(&mutexEtat);

    if (status == STATUS_OK) {
		rt_mutex_acquire(&mutexRobot,TM_INFINITE);
		status = robot->start(robot); // remplace start_insecurely(robot)  
	    rt_mutex_release(&mutexRobot);

		rt_printf("tconnect : status=%d",status);
	    if (status == STATUS_OK){
			rt_printf("tconnect : Robot démarrer\n");
    	    if(err = rt_task_spawn(&tcomm,NULL,0,PRIORITY_TCOMM,0,&wtd_ctrl_comm_sup_rob,NULL)){
		        rt_printf("Error task spawn: %s\n", strerror(-err));
		        exit(EXIT_FAILURE);
    	   }
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
  int size = 1;
  int num_msg = 0;

  while(1){
    rt_printf("tserver : Début de l'exécution de serveur\n");
		
		rt_mutex_acquire(&mutexServeur,TM_INFINITE);
	    serveur->open(serveur, "8000");
	    rt_mutex_release(&mutexServeur);
      

    rt_printf("tserver : Connexion\n");

    rt_mutex_acquire(&mutexEtat, TM_INFINITE);
    etatCommMoniteur = 0;
    rt_mutex_release(&mutexEtat);

    while (size > 0) {
      rt_printf("tserver : Attente d'un message\n");
    	//rt_mutex_acquire(&mutexServeur,TM_INFINITE);	  
		//rt_printf("On a pris le mutexServeur pour serveur->receive\n");
		size = serveur->receive(serveur, msg);
	    //rt_mutex_release(&mutexServeur);
    
  	num_msg++;
      if (size > 0) {
	switch (msg->get_type(msg)) {
	case MESSAGE_TYPE_ACTION:
	  rt_printf("tserver : Le message %d reçu est une action\n",num_msg);
	  DAction *action = d_new_action();
	  action->from_message(action, msg);
	  switch (action->get_order(action)) {
	  case ACTION_CONNECT_ROBOT:
	    rt_printf("tserver : Action connecter robot\n");
	    rt_sem_v(&semConnecterRobot);
	    break;
	  case ACTION_COMPUTE_CONTINUOUSLY_POSITION:
		rt_printf("tserver : Action compute position\n");
		
	  }
	  break;
	case MESSAGE_TYPE_MOVEMENT:
	  rt_printf("tserver : Le message reçu %d est un mouvement\n",num_msg);
	  rt_mutex_acquire(&mutexMove, TM_INFINITE);
	  move->from_message(move, msg);
	  move->print(move);
	  rt_mutex_release(&mutexMove);
	  break;
	}
		   	 
      }
    }
    /* cas d'une perte de communication entre le moniteur de le superviseur */
    /* on se remet en attente d'un message de connexion envoyé par le moniteur --> on remet size > 0 pour pouvoir rentrer une premiere fois dans le while(size>0) */
    size = 1;
  }
}

void deplacer(void *arg) {
  int status = 1;
  int gauche;
  int droite;
  DMessage *message;

  rt_printf("tmove : Debut de l'éxecution de periodique à 1s\n");
  rt_task_set_periodic(NULL, TM_NOW, 1000000000);

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
      
	  rt_mutex_acquire(&mutexRobot,TM_INFINITE);
      status = robot->set_motors(robot, gauche, droite);
	  rt_mutex_release(&mutexRobot);

      if (status != STATUS_OK) {
	/* erreur de communication entre le superviseur et le robot */
	rt_mutex_acquire(&mutexCptErrors, TM_INFINITE);
	compteur_errors++;
	rt_mutex_release(&mutexCptErrors);

	
   }else {
	/* communication reussie entre le superviseur et le robot */
	rt_mutex_acquire(&mutexCptErrors, TM_INFINITE);
	compteur_errors=0;
	rt_mutex_release(&mutexCptErrors);
      }
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
