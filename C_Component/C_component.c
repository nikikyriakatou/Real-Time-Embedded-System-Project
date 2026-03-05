#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <math.h>
#include <time.h>
#include <mosquitto.h>
#include <string.h>

#define MAX_MSG_LEN 100

#define MQTT_HOSTNAME "localhost"
#define MQTT_PORT 10000
#define MQTT_CLIENTID "project"
#define MQTT_BASE_CORD_X "base/baseA/missile_x"  //you add extra topics after.
#define MQTT_BASE_CORD_Y "base/baseA/missile_y"
#define MQTT_BASE_SIGNAL "base/baseA/signal"
#define MQTT_BASE_CORD "base/baseA/cords"


int signal_received=0;
float root;  

struct msg_buffer {
    long msg_type;
    float missile_x;
    float missile_y;
    float missile_speed;
};

void message_rcv(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message) {

    if(message->payloadlen) {
        printf("%s %s\n", message->topic, (char *)message->payload);
        signal_received=1;
    } else {
        printf("%s (null)\n", message->topic);
    }
}



float root_finder(float a, float b, float c) { //added c in case we want to have it later but serves no purpose now.
    float discriminant, root1, root2;

    // Calculate the discriminant-Δ
    discriminant = b * b - 4 * a * c;

    // Check the category of the root belonging
    if (discriminant > 0) {
        root1 = (-b + sqrt(discriminant)) / (2 * a);
        root2 = (-b - sqrt(discriminant)) / (2 * a);
        printf("Roots are real and distinct: %.2f and %.2f\n", root1, root2);
    } else if(discriminant == 0){
        // Real and equal roots
        root1 = root2 = -b / (2 * a);
        printf("Roots are real and equal: %.2f\n", root1);
    } else {
        printf("No real roots\n");
    }

    return root2;
}

void generate_missile_cord_data(float traj_x_var, float traj_y_var, float starting_missile_speed, int message_key) {
    float a = traj_x_var;
    float b = traj_y_var;
    float f_derivative=0.0;
    double f=0.0;
    double delta=0.000004;

    struct msg_buffer message;

    float x = starting_missile_speed;
    
    while (1) {
        sleep(0.01);
        x = x - ((2.0 / 10) * x) - delta;  //added a delta otherwise it stalls in a plateau after a lot of iterations.
        f = ((-a) * (pow(x, 2))) + (b * x);
        f_derivative= (-2.0*a*x+b);  //derivativy of the function is speed.

        float x_round = roundf(x * 100000) / 100000; // Round to five decimal points otherwise its too big of precision
        float f_round = roundf(f * 100000) / 100000; 
        float f_derivative_round = roundf(f_derivative * 100000) / 100000;
        x=x_round;
        f=f_round;
        f_derivative=fabs(f_derivative_round);

        printf("Missile position: x = %f, y = %f\n", x, f);

        message.msg_type = 1;
        message.missile_x = x;
        message.missile_y = f;
        message.missile_speed= f_derivative;

        // Send the message to mq
        if (msgsnd(message_key, &message, sizeof(message) - sizeof(long), 0) == -1) {
            perror("msgsnd");
            exit(1);
        }

        // Break and end generation of cords
        if (x <= 0.0 && f <= 0.0) {
            x=0.0;
            f=0.0;
            printf("Missile has hit the base.\n");
            break;
        }

        
        
    }
}

int main() 
{
    char mqtt_msgx[50];
    char mqtt_msgy[50];
    char jsonstring[1000];
    //Mosquitto initialization , have to use -lmosquitto for the library
    int rc;
    struct mosquitto *mosq=NULL;
    
    mosquitto_lib_init();
       
 

    mosq=mosquitto_new(MQTT_CLIENTID,true,NULL);
    if(!mosq)
    {
        printf("Cant initiallize mosquitto library\n");
        exit(-1);
    }

    mosquitto_message_callback_set(mosq, message_rcv);

    rc=mosquitto_connect(mosq,MQTT_HOSTNAME,MQTT_PORT,60);  //connects to 10000 port
    if(rc!=0)
    {
        printf("Client could not connect to broker! Error Code: %d\n", rc);
        mosquitto_destroy(mosq);
        exit(0);
    }
    printf("Connected to broker!\n");
 

    rc = mosquitto_subscribe(mosq, NULL, MQTT_BASE_SIGNAL, 0);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Unable to subscribe to topic: %d\n", rc);
        return 1;
    }

      while (!signal_received) {
        mosquitto_loop(mosq, -1, 1);
    }

   


    int msgid;
    int missile_id=0;
    float var_a = 1.9;
    float var_b = 2.6;  //these can be dynamically changed but are constant for now.
    float missile_speed = 1;

    root = root_finder((-var_a), var_b, 0.0);

    printf("Starting missile position: %.2f\n", root);

    key_t key = ftok(".", 'A'); // Use current directory as the key path
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget");
        exit(1);
    }

    pid_t missile_process = fork();
    missile_id=missile_process;

    if (missile_process < 0) {
        perror("fork fail");
        exit(1);
    } else if (missile_process == 0) {
        printf("Child process started\n");
        generate_missile_cord_data(var_a, var_b, root, msgid);
        exit(EXIT_SUCCESS);
    } else {
        printf("Parent process started\n");
        wait(NULL); // Wait for the child process to finish
        struct msg_buffer received_message;

        // Parent
        while (1) {
            // Receive the messages
            ssize_t msg_length = msgrcv(msgid, &received_message, sizeof(received_message) - sizeof(long), 1, IPC_NOWAIT);
            sleep(1);  //Sleep while the child process calculated the trajectory of the missile 
            if (msg_length == -1) {
                if (errno == ENOMSG) { // Check if there are messages in the msg queue
                    
                    printf("No more messages in the queue. Waiting for child process to calculate the coordinates.\n");
                
                    break;
                } else {
                    perror("msgrcv");
                    exit(1);
                }
            } else {
                // Process the received message
                printf("Received message: %d missile position - x = %.5f, y = %.5f with speed %.2f km/s\n",missile_id, received_message.missile_x, received_message.missile_y,received_message.missile_speed);
                snprintf(mqtt_msgx, sizeof(mqtt_msgx), "%.2f", received_message.missile_x);
               

                snprintf(mqtt_msgy, sizeof(mqtt_msgy), "%.2f", received_message.missile_y);
         
                //publish the json message containing all the data to the according topic
                sprintf(jsonstring,"{\"Author\": \"baseA\",\"Content\": { \"CoordX\": %s, \"CoordY\": %s, \"MissileId\": %d, \"Outpost\": \"baseA\", \"Speed\": %f }}++"
                ,mqtt_msgx,mqtt_msgy,missile_id,received_message.missile_speed);
           
                mosquitto_publish(mosq, NULL, MQTT_BASE_CORD, sizeof(jsonstring),jsonstring, 0, false);
            }
           
        }

        // Remove message queue
        if (msgctl(msgid, IPC_RMID, NULL) == -1) {
            perror("msgctl");
            exit(1);
        } else {
            printf("Message queue removed successfully.\n");
        }
    
    }
    
        


    if(mosquitto_disconnect(mosq)==MOSQ_ERR_SUCCESS){
        printf("Mosquitto disconneted\n");}
    
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();



    return 0;
}
