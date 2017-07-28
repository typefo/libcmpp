#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <cmpp.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("usage: %s <phone> <message>\n", argv[0]);
        return 0;
    }
    
    int err;
    cmpp_sp_t cmpp;
    cmpp_pack_t pack;
    
    signal(SIGPIPE, SIG_IGN);

    char *host = "192.168.1.100";
    int port = 7890;
    char *user = "901234";
    char *password = "123456";

    /* Cmpp Socket Initialization */
    err = cmpp_init_sp(&cmpp, host, port);
    if (err) {
        printf("can't connect to %s server\n", host);
        return -1;
    }

    printf("connect to %s server successfull\n", host);

    /* Cmpp Login */
    err = cmpp_connect(&cmpp.sock, "901234", "123456");
    if (err) {
        fprintf(stderr, "send cmpp_connect error\n");
        goto exit;
    }

    /* check login status */
    err = cmpp_recv(&cmpp.sock, &pack, sizeof(pack));
    if (err) {
        if (err == -1) {
            printf("close connection be from server\n");
            return -1;
        }

        printf("cmpp cmpp_recv() failed\n");
        return -1;
    }

    if (cmpp_check_method(&pack, sizeof(pack), CMPP_CONNECT_RESP)) {
        unsigned char status;
        cmpp_pack_get_integer(&pack, cmpp_connect_resp_status, &status, 1);
        switch (status) {
        case 0:
            printf("cmpp login successfull\n");
            break;
        case 1:
            lamb_errlog(config.logfile, "Incorrect protocol packets", 0);
            return -1;
        case 2:
            lamb_errlog(config.logfile, "Illegal source address", 0);
            return -1;
        case 3:
            lamb_errlog(config.logfile, "Authenticator failed", 0);
            return -1;
        case 4:
            lamb_errlog(config.logfile, "The protocol version is too high", 0);
            return -1;
        default:
            lamb_errlog(config.logfile, "Unknown error", 0);
            return -1;
        }
    } else {
        printf("The server response packet cannot be resolved\n");
        return -1;
    }
    
    sleep(1);

    /* Message Receive Number */
    char *phone = argv[1];

    /* Message Content */
    char *message = argv[2];

    /* Delivery Report */
    bool delivery = false;

    /* Service Code */
    char *serviceId = "1065860008";

    /* Message Character Encoding */
    char *msgFmt = "UCS-2";

    /* Enterprise Number */
    char *msgSrc = user;

    /* Cmpp Send Message */
    err = cmpp_submit(&cmpp.sock, phone, message, delivery, serviceId, msgFmt, msgSrc);
    if (err) {
        fprintf(stderr, "cmpp cmpp_submit error\n");
        goto exit;
    }
    
    printf("send message cmpp_submit successfull\n");
    sleep(1);
    
    /* Cmpp Logout */
    printf("send cmpp_terminate to cmpp server\n");
    cmpp_terminate(&cmpp.sock);

    sleep(1);

exit:
    /* Close Cmpp Socket Connect */
    printf("closing server connection\n");
    cmpp_sp_close(&cmpp);
    return 0;
}
