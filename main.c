/* SWE-699 - TERM PROJECT - SPRING 2023
 * Main.c
 * Author: Vy Phan
 * Created on: 04/24/2023
 * Last Updated: 05/03/2023
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <sched.h>

#include "lib/gpio.h"
#include "lib/i2c_bbb.h"
#include "lib/tea5767_i2c_driver.h"

#define BUTTON_WAIT 10 // ms
#define USEC_PER_MS 1000 // 1000us = 1ms

#define FM_MODULE_ADDR 		 		  0x60
#define RADIO_AUDIO_BUTTON 	 		  P9_24
#define TOGGLE_DIGIT_BUTTON    	      P9_26
#define FREQUENCY_TUNE_BACK_BUTTON    P9_18
#define FREQUENCY_TUNE_FORWARD_BUTTON P9_27
#define RADIO_TUNE_BUTTON			  P9_12

#define HIGHER_PRIO 20
#define LOWER_PRIO  50

static void* fmThreadFunc	 	   		 (void* arg); 
static void* displayThreadFunc	   		 (void* arg);
static void* audioButtonThreadFunc 		 (void* arg);
static void* toggleDigitButtonThreadFunc (void* arg);
static void* backButtonThreadFunc 	     (void* arg);
static void* forwardButtonThreadFunc     (void* arg);
static void* tuneButtonThreadFunc        (void* arg);


// Mutex
/* To lock the global tuned frequency */
static pthread_mutex_t freq_mutex  = PTHREAD_MUTEX_INITIALIZER;
/* To lock the global variable of mute/unmute */
static pthread_mutex_t audio_mutex = PTHREAD_MUTEX_INITIALIZER;
/* To lock the global variable of standby mode*/
static pthread_mutex_t digit_mutex  = PTHREAD_MUTEX_INITIALIZER;
/* To lock global variable of search signal*/
static pthread_mutex_t flag_mutex  = PTHREAD_MUTEX_INITIALIZER;
/* To lock global variable of lcd update */
static pthread_mutex_t lcd_update_mutex = PTHREAD_MUTEX_INITIALIZER;

// Condition variable
/* To signal that FM module to check the global flag for next action*/
static pthread_cond_t check_flag_cond 	  = PTHREAD_COND_INITIALIZER;
/* To signal LCD display to update*/
static pthread_cond_t lcd_update_cond = PTHREAD_COND_INITIALIZER;

typedef enum Flag {
	AUDIO,
	TUNE,
	WAIT
} Flag;


// Global variables
static float g_frequency = DEFAULT_FREQ;	// The tuned frequency
static uint8_t g_audio = 1;		// if audio = 0, then mute.
							    // if audio = 1, then unmute.
static Flag g_flag = WAIT;		// Default flag is WAIT, or do nothing
static uint8_t g_lcd_update = 0;   // If lcd_update = 1, update the display; else, do nothing
static uint8_t g_digit = 0;		// If digit = 0, then modify the decimal digit; else left-most value


int main() {

	/* Set up the GPIO */
	GPIO_SetDirection(TOGGLE_DIGIT_BUTTON, 			 GPIO_IN);
	GPIO_SetDirection(RADIO_AUDIO_BUTTON, 			 GPIO_IN);
	GPIO_SetDirection(FREQUENCY_TUNE_BACK_BUTTON, 	 GPIO_IN);
	GPIO_SetDirection(FREQUENCY_TUNE_FORWARD_BUTTON, GPIO_IN);
	GPIO_SetDirection(RADIO_TUNE_BUTTON, 			 GPIO_IN);

     // Thread attributes
    struct sched_param hParam;
    struct sched_param lParam;
    pthread_attr_t hAttr;
    pthread_attr_t lAttr;
    
    // Initialize the attributes
    (void) pthread_attr_init (&hAttr);
    (void) pthread_attr_init (&lAttr);
    // Get the schedule param from the attribute
    (void) pthread_attr_getschedparam (&hAttr, &hParam);
    (void) pthread_attr_getschedparam (&lAttr, &lParam);
    // Modify the priorities
    hParam.sched_priority = HIGHER_PRIO;
    lParam.sched_priority = LOWER_PRIO;
    // Set the attributes with the new params 
    (void) pthread_attr_setschedparam (&hAttr, &hParam);
    (void) pthread_attr_setschedparam (&lAttr, &lParam);
    // Set the scheduling policy to real-time FIFO
    (void) pthread_attr_setschedpolicy(&hAttr, SCHED_FIFO);
    (void) pthread_attr_setschedpolicy(&lAttr, SCHED_FIFO);


    pthread_t fm_thread;
    pthread_t display_thread;
    pthread_t audio_button_thread;
    pthread_t toggle_digit_button_thread;
    pthread_t back_button_thread;
    pthread_t forward_button_thread;
    pthread_t tune_button_thread;

	(void) pthread_create(&fm_thread, &lAttr, &fmThreadFunc, NULL);
	(void) pthread_create(&display_thread, &lAttr, &displayThreadFunc, NULL);
	(void) pthread_create(&audio_button_thread, &hAttr, &audioButtonThreadFunc, NULL);
	(void) pthread_create(&toggle_digit_button_thread, &hAttr, &toggleDigitButtonThreadFunc, NULL);
	(void) pthread_create(&back_button_thread, &hAttr, &backButtonThreadFunc, NULL);
	(void) pthread_create(&forward_button_thread, &hAttr, &forwardButtonThreadFunc, NULL);
	(void) pthread_create(&tune_button_thread, &hAttr, &tuneButtonThreadFunc, NULL);



	(void) pthread_join(fm_thread, NULL);
	(void) pthread_join(display_thread, NULL);
	(void) pthread_join(audio_button_thread, NULL);
	(void) pthread_join(toggle_digit_button_thread, NULL);
	(void) pthread_join(back_button_thread, NULL);
	(void) pthread_join(forward_button_thread, NULL);
	(void) pthread_join(tune_button_thread, NULL);

	// Destroy mutex
    (void) pthread_mutex_destroy(&freq_mutex); 
    (void) pthread_mutex_destroy(&audio_mutex);
    (void) pthread_mutex_destroy(&digit_mutex);
    (void) pthread_mutex_destroy(&flag_mutex);
    (void) pthread_mutex_destroy(&lcd_update_mutex);

    // Destroy condition variables
    (void) pthread_cond_destroy(&check_flag_cond);
    (void) pthread_cond_destroy(&lcd_update_cond);


	return 0;
}


/****************************************************************
 * Function Name : fmThreadFunc
 * Description   : The thread function for the FM module
 * Returns       : N/A
 * Params        @arg : arguments of the thread function
 ****************************************************************/
static void* fmThreadFunc (void* arg)
{
	/* Open the I2C bus at i2c_2 */
	int i2c_bus;	// File descriptor for I2C bus
	if (I2C_OpenBus(&i2c_bus, I2C_2_DEV_PATH) < 0)
	{
		perror("ERROR: FmThreadFunc - Failed to open I2C bus");
		return NULL;
	}

	/* Create an FM module */
	TEA5767_FM_module fm_device;
	fm_device.i2c_bus = &i2c_bus;
	fm_device.device_addr = FM_MODULE_ADDR;

	/* Initialize the fm module and tune to its default frequency*/
	if (TEA5767_Init(&fm_device) < 0)
	{
		perror("ERROR: FmThreadFunc - Failed to init the FM module");
		return NULL;
	}

	/* Inifity loop starts */
	while (1)
	{
		(void) pthread_mutex_lock(&flag_mutex);
		while (g_flag == WAIT)
		{
			(void) pthread_cond_wait(&check_flag_cond, &flag_mutex);
		}

		/* Check for which flag it is */
		switch(g_flag)
		{
		/* TUNE flag: tell fm module to tune to the frequency */
		case TUNE:
			(void) pthread_mutex_lock(&freq_mutex);
			if (TEA5767_SetFrequency(&fm_device, g_frequency) < 0)
			{
				perror("ERROR: FmThreadFunc - Failed to set the frequency.");
			}
			(void) pthread_mutex_unlock(&freq_mutex);
			break;

		/* AUDIO flag: tell fm module to mute or unmute the audio */
		case AUDIO:
			(void) pthread_mutex_lock(&audio_mutex);
			if (g_audio == 0)
			{
				/* Mute the audio */
				if (TEA5767_Mute(&fm_device) < 0)
				{
					perror("ERROR: FmThreadFunc - Failed to mute the audio.");
				}
			}
			else
			{
				/* Unmute the audio */
				if (TEA5767_Unmute(&fm_device) < 0)
				{
					perror("ERROR: FmThreadFunc - Failed to unmute the audio.");
				}
			}
			(void) pthread_mutex_unlock(&audio_mutex);
			break;

		/* Default case */
		default:
			printf("INFO: FmThreadFunc - Default case - Do nothing.");
		} // End of switch case

		/* Reset the flag to WAIT */
		g_flag = WAIT;
		(void) pthread_mutex_unlock(&flag_mutex);

		/* Signal display to update */
		(void) pthread_mutex_lock(&lcd_update_mutex);
		g_lcd_update = 1;
		(void) pthread_mutex_unlock(&lcd_update_mutex);
		(void) pthread_cond_signal(&lcd_update_cond);


	} // End of inifity loop

}// End of fmThreadFunc

/****************************************************************
 * Function Name : displayThreadFunc
 * Description   : The thread function for the character LCD display
 * Returns       : N/A
 * Params        @arg : arguments of the thread function
 ****************************************************************/
static void* displayThreadFunc (void* arg)
{
	/* Terminal display at beginning */
	/* Format line 1 with tuned Frequency */
	/* Print Project header */
	printf("--- FM RADIO RECEIVER --\n");
	printf("--- 87.5MHz - 108MHz ---\n");
	printf("-------- Vy Phan -------\n");
	printf("------------------------\n");
	(void) pthread_mutex_lock(&freq_mutex);
	printf("Frequency: %.1f\n", g_frequency);
	(void) pthread_mutex_unlock(&freq_mutex);
	/* Format line 2 with audio status */
	(void) pthread_mutex_lock(&audio_mutex);
	printf("Audio: %s\n", g_audio ? "ON" : "MUTED");
	(void) pthread_mutex_unlock(&audio_mutex);
	printf("------------------------\n");

	
	/* Inifity loop starts */
	while (1)
	{
		/* Check for any lcd update signal */
		(void) pthread_mutex_lock(&lcd_update_mutex);
		while (g_lcd_update == 0)
		{
			(void) pthread_cond_wait(&lcd_update_cond, &lcd_update_mutex);
		}
		/* Reset the lcd update signal */
		g_lcd_update = 0;
		(void) pthread_mutex_unlock(&lcd_update_mutex);

		/* Update the LCD display with the two formatted lines */
		printf("\n------------------------\n");
		(void) pthread_mutex_lock(&freq_mutex);
		printf("Frequency: %.1f\n", g_frequency);
		(void) pthread_mutex_unlock(&freq_mutex);
		/* Format line 2 with audio status */
		(void) pthread_mutex_lock(&audio_mutex);
		printf("Audio: %s\n", g_audio ? "ON" : "MUTED");
		(void) pthread_mutex_unlock(&audio_mutex);
        printf("------------------------\n");
	} // End of inifity loop
    return NULL;
}




/****************************************************************
 * Function Name : audioButtonThreadFunc
 * Description   : listen to a button press action and toggle
 * 					the muted/unmuted state of the audio
 * Returns       : 0 on success, -1 on failure
 * Params        : N/A
 ****************************************************************/
static void* audioButtonThreadFunc (void* arg)
{
	int8_t isPressed = 0; // current button's state
    int8_t lastState = 0; // last button's state
    while (1)
    {
        /* Read the current button's state from GPIO */
        isPressed = GPIO_ReadValue(RADIO_AUDIO_BUTTON);
        if (isPressed < 0)
        {
        	perror("ERROR: audioButtonThreadFunc - Failed to read the button press.");
        	return NULL;
        }
        /* If the button is released and the last
         * button's state is pressed, we have a complete
         * button-press action */
        if (!isPressed && lastState)
        {
            (void) pthread_mutex_lock (&audio_mutex);
            if (g_audio == 1)
            {
                /* If audio is on, then mute */
                g_audio = 0;
            }
            else
            {
                /* If audio is muted, then unmute */
                g_audio = 1;

            }
            (void) pthread_mutex_unlock(&audio_mutex);

	        /* Change flag and send a signal to check flag*/
	        (void) pthread_mutex_lock (&flag_mutex);
	        g_flag = AUDIO;
	        (void) pthread_mutex_unlock (&flag_mutex);
	        (void) pthread_cond_signal (&check_flag_cond);
        }
        /* Update the last button's state with the current */
        lastState = isPressed;
        /* Sleep for 10ms */
        (void) usleep (BUTTON_WAIT * USEC_PER_MS);
    }
    return NULL;
}


/****************************************************************
 * Function Name : modeButtonThreadFunc
 * Description   : listen to a button press action and toggle
 * 					the modification location on the freq value
 * Returns       : 0 on success, -1 on failure
 * Params        : N/A
 ****************************************************************/
static void* toggleDigitButtonThreadFunc (void* arg)
{
	uint8_t isPressed = 0; // current button's state
    uint8_t lastState = 0; // last button's state
    while (1)
    {
        /* Read the current button's state from GPIO */
        isPressed = GPIO_ReadValue(TOGGLE_DIGIT_BUTTON);
        if (isPressed < 0)
        {
        	perror("ERROR: toggleDigitButtonThreadFunc - Failed to read the button press.");
        	return NULL;
        }        
        /* If the button is released and the last
         * button's state is pressed, we have a complete
         * button-press action */
        if (!isPressed && lastState)
        {
            (void) pthread_mutex_lock(&digit_mutex);
            if (g_digit == 1)
            {
                /* Set the modification location on the "tenths" value*/
                g_digit = 0;
            }
            else
            {
                /* Set the modification location on the "ones" value */
                g_digit = 1;

            }
            (void) pthread_mutex_unlock(&digit_mutex);
        }
        /* Update the last button's state with the current */
        lastState = isPressed;
        /* Sleep for 10ms */
        (void) usleep (BUTTON_WAIT * USEC_PER_MS);
    }
    return NULL;
}

/****************************************************************
 * Function Name : backButtonThreadFunc
 * Description   : listen to a button press action and decrease 
 * 					the value of frequency
 * Returns       : 0 on success, -1 on failure
 * Params        : N/A
 ****************************************************************/
static void* backButtonThreadFunc (void* arg)
{
	uint8_t isPressed = 0; // current button's state
    uint8_t lastState = 0; // last button's state
    while (1)
    {
        /* Read the current button's state from GPIO */
        isPressed = GPIO_ReadValue(FREQUENCY_TUNE_BACK_BUTTON);
        if (isPressed < 0)
        {
        	perror("ERROR: backButtonThreadFunc - Failed to read the button press.");
        	return NULL;
        }        
        /* If the button is released and the last
         * button's state is pressed, we have a complete
         * button-press action */
        if (!isPressed && lastState)
        {
            (void) pthread_mutex_lock(&digit_mutex);
            (void) pthread_mutex_lock(&freq_mutex);
            if (g_digit == 1)
            {
                if (g_frequency > 88.4)
                /* Decrease the frequency value by one */
            	{
                    g_frequency -= 1.0;
                }
            }
            else
            {
                if (g_frequency > 87.5)
                /* Decrease the frequency value by 0.1 */
                {
                    g_frequency -= 0.1;
                }

            }
            printf("\rTuning Frequency: %.1f", g_frequency);
            fflush(stdout);
            (void) pthread_mutex_unlock(&freq_mutex);
            (void) pthread_mutex_unlock(&digit_mutex);
        }
        /* Update the last button's state with the current */
        lastState = isPressed;
        /* Sleep for 10ms */
        (void) usleep (BUTTON_WAIT * USEC_PER_MS);
    }
    return NULL;
}
/****************************************************************
 * Function Name : forwardButtonThreadFunc
 * Description   : listen to a button press action and increase 
 * 					the value of frequency
 * Returns       : 0 on success, -1 on failure
 * Params        : N/A
 ****************************************************************/
static void* forwardButtonThreadFunc (void* arg)
{
	uint8_t isPressed = 0; // current button's state
    uint8_t lastState = 0; // last button's state
    while (1)
    {
        /* Read the current button's state from GPIO */
        isPressed = GPIO_ReadValue(FREQUENCY_TUNE_FORWARD_BUTTON);
        if (isPressed < 0)
        {
        	perror("ERROR: forwardButtonThreadFunc - Failed to read the button press.");
        	return NULL;
        }        
        /* If the button is released and the last
         * button's state is pressed, we have a complete
         * button-press action */
        if (!isPressed && lastState)
        {
            (void) pthread_mutex_lock(&digit_mutex);
            (void) pthread_mutex_lock(&freq_mutex);
            if (g_digit == 1)
            {
                if (g_frequency < 107.0)
                /* Increase the frequency value by one */
            	{
                    g_frequency += 1.0;
                }
            }
            else
            {
                if (g_frequency < 107.9)
                /* Increase the frequency value by 0.1 */
                {
                    g_frequency += 0.1;
                }
            }
            printf("\rTuning Frequency: %.1f", g_frequency);
            fflush(stdout);
            (void) pthread_mutex_unlock(&freq_mutex);
            (void) pthread_mutex_unlock(&digit_mutex);
        }
        /* Update the last button's state with the current */
        lastState = isPressed;
        /* Sleep for 10ms */
        (void) usleep (BUTTON_WAIT * USEC_PER_MS);
    }
    return NULL;
}
/****************************************************************
 * Function Name : tuneButtonThreadFunc
 * Description   : listen to a button press action and signal the
 * 					FM Module to tune
 * Returns       : 0 on success, -1 on failure
 * Params        : N/A
 ****************************************************************/
 static void* tuneButtonThreadFunc (void* arg)
{
	uint8_t isPressed = 0; // current button's state
    uint8_t lastState = 0; // last button's state
    while (1)
    {
        /* Read the current button's state from GPIO */
        isPressed = GPIO_ReadValue(RADIO_TUNE_BUTTON);
        if (isPressed < 0)
        {
        	perror("ERROR: tuneButtonThreadFunc - Failed to read the button press.");
        	return NULL;
        }        
        /* If the button is released and the last
         * button's state is pressed, we have a complete
         * button-press action */
        if (!isPressed && lastState)
        {
            (void) pthread_mutex_lock(&flag_mutex);
            g_flag = TUNE;
            (void) pthread_cond_signal(&check_flag_cond);
            (void) pthread_mutex_unlock(&flag_mutex);
        }
        /* Update the last button's state with the current */
        lastState = isPressed;
        /* Sleep for 10ms */
        (void) usleep (BUTTON_WAIT * USEC_PER_MS);
    }
    return NULL;

}