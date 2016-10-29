#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>	/* Defines O_* constants  */
#include <sys/stat.h>	/* Defines mode constants */
#include <sys/mman.h>
#include <sys/types.h>

#include "mailbox_toolkit.h"

mailbox_t mailbox_open(int id)
{
    int fd;
    size_t size = sizeof(Mail_box_t) + (BUFFER_SIZE * sizeof(mail_t));
    void *addr;
    struct stat sb;

    /*	Mailbox id name  */
    char mailbox_id[15];
    sprintf(mailbox_id, "__mailbox_%d", id);

    /*  Create shared memory object, O_RDWR for ftruncate  */
    fd = shm_open(mailbox_id, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1){
        printf("%s shm_open() create failed\n",mailbox_id);
        return NULL;
    }

    /*  Resize the object */
    if (ftruncate(fd,size) == -1){
        printf("%s truncate failed\n",mailbox_id);
        return NULL;
    }

    addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (addr == MAP_FAILED){
        printf("%s MMaping failed\n",mailbox_id);
        return NULL;
    }
    
    if (close(fd) == -1){
	printf("Fd closing failure\n");
	return NULL;
    }

    /* Return the address space */
    return addr;
}

int mailbox_unlink(int id)
{
    char mailbox_id[15];
    sprintf(mailbox_id, "__mailbox_%d", id);
    if (shm_unlink(mailbox_id) == -1)
	return -1;
    return 0;
}

int mailbox_close(mailbox_t box)
{
    Mail_box_t *curBox = (Mail_box_t*)box;
    if (munmap(curBox, sizeof(curBox)) == -1){
	printf("Munmap failed!\n");
	return -1;
    }
    free(curBox);
    return 0;
}

int mailbox_send(mailbox_t box, mail_t *mail)
{
    Mail_box_t *curBox = (Mail_box_t*)box;
    
    /*  Mailbox fulled  */
    if(((curBox->in + 1) % BUFFER_SIZE) == curBox->out)
	return -1;

    /* Copying mail to mail box */
    curBox->mail[curBox->in].from = mail->from;
    curBox->mail[curBox->in].type = mail->type;
    memcpy(curBox->mail[curBox->in].sstr, mail->sstr, sizeof(mail->sstr));
    memcpy(curBox->mail[curBox->in].lstr, mail->lstr, sizeof(mail->lstr));

    curBox->in = (curBox->in + 1) % BUFFER_SIZE;
    return 0;
}

int mailbox_recv(mailbox_t box, mail_t *mail)
{
    Mail_box_t *curBox = (Mail_box_t*)box;

    /* Mail box is empty */
    if(curBox->in == curBox->out)
	return -1;
    
    /* Copying mail from mail box */
    mail->from = curBox->mail[curBox->out].from;
    mail->type = curBox->mail[curBox->out].type;
    memcpy(mail->sstr, curBox->mail[curBox->out].sstr, sizeof(curBox->mail[curBox->out].sstr));
    memcpy(mail->lstr, curBox->mail[curBox->out].lstr, sizeof(curBox->mail[curBox->out].lstr));

    curBox->out = (curBox->out + 1) % BUFFER_SIZE;
    return 0;
}

int mailbox_check_empty(mailbox_t box)
{
    Mail_box_t *curBox = (Mail_box_t*)box;
    
    if(curBox->in == curBox->out)
	return 1;
    return 0;
}

int mailbox_check_full(mailbox_t box)
{
    Mail_box_t *curBox = (Mail_box_t*)box;

    if(((curBox->in + 1) % BUFFER_SIZE) == curBox->out)
	return 1;
    return 0;
}
