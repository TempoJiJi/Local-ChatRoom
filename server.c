
/*  Server will keep running untill an interupt signal is received		    *
 *  "Ctrl+C" will close the server gracefully,  and unlink the shared memory	    *
 *  "Ctrl+Z" will close the server immediately, and wouldn't unlink the shared mem  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>	/* Defines O_* constants  */
#include <sys/stat.h>	/* Defines mode constants */
#include <sys/mman.h>
#include <sys/types.h>
#include <pthread.h>

#include "mailbox_toolkit.h"
#include "server.h"

chat_room room[MAX_MEMBER];
static volatile int shutdown = 1;

/* "Ctrl+C" signal handler */
void handler(){
    while(1){
	if (!mailbox_unlink(0))
	    break;
    }
    shutdown = 0;
}

int client_broadcast(mail_t *mail, int sender) 
{
    /* Send mail to other clients in the room */
    for(int i=1; i<MAX_MEMBER; i++) {
	if (room[i].client_box && i != sender){ 
	    /* Wait if mailbox is fulled */
	    while (mailbox_check_full(room[i].client_box)); 
	    mailbox_send(room[i].client_box, mail);
	}
    }
}

int main(int argc,char *argv[])
{
    Mail_box_t *server_box,*box;
    mail_t *mail;
    mail = malloc(sizeof(mail_t));

    signal(SIGINT,handler);

    /* Initialize chat room */
    memcpy(room[0].name, "SERVER", 6);
    for (int i=1; i<MAX_MEMBER; i++)
	room[i].client_box = NULL;

    /* Create server mail box */
    server_box = (Mail_box_t*)mailbox_open(0);
    server_box->in = 0;
    server_box->out = 0;

    printf( "Server create successful!\n"
	    "Enter \"Ctrl+C\" to shutdown the server!\n");

    while (shutdown) {
	printf("Listening...\n");
	/* Wait for mail come in and get the mail */
	while (mailbox_check_empty(server_box) && shutdown);
    
	if (!shutdown)
	    break;

	mailbox_recv(server_box, mail);

	/* Create or opens an existing box from the share memory */
	box = (Mail_box_t*)mailbox_open(mail->from);

	switch (mail->type) {
	    case JOIN:
		printf("Join processing...\n");
		    room[mail->from].client_box = box;
		    memcpy(room[mail->from].name, mail->sstr, sizeof(mail->sstr));
		    sprintf(mail->lstr, "%s has joined the room!", mail->sstr); 
		    mail->type = SERVER;	// Mark the mail sent by server
		    client_broadcast(mail,mail->from);
		break;

	    case BROADCAST:
		printf("Broadcasting...\n");
		    client_broadcast(mail, mail->from);
		break;

	    case LEAVE:
		printf("Leaving processing...\n");
		    room[mail->from].client_box = NULL;
		    sprintf(mail->lstr, "%s has left the room!", mail->sstr);
		    mail->type = SERVER;	//Mark the mail sent by server
		    client_broadcast(mail, mail->from);
		    mailbox_unlink(mail->from);
		break;

	    case LIST:
		printf("List processing...\n");
		    mail->type = SERVER;	//Mark the mail sent by server
		    for (int i=0; i<MAX_MEMBER; i++) {
			/* List one by one */
			if(room[i].client_box && i != mail->from){
			    sprintf(mail->lstr, "%s is inside the room!", room[i].name);
			    /* Wait if mailbox is fulled */
			    while (mailbox_check_full(box));
			    mailbox_send(box, mail);
			}
		    }
		break;

	    case WHISPER:
		printf("Whisper processing...\n");
		    Mail_box_t *reciever;
		    for (int i=0; i<MAX_MEMBER; i++) {
			if(strcmp(room[i].name, mail->sstr) == 0){
			    reciever = room[i].client_box;
			    break;
			}
		    }
		    memcpy(mail->sstr, room[mail->from].name, sizeof(room[mail->from].name));
		    /* Wait if mailbox is fulled */
		    while (mailbox_check_full(reciever)); 
		    mailbox_send(reciever, mail);
		break;
	}
    }
    printf("\nServer shut down completely!\n");
    return 0;
}
