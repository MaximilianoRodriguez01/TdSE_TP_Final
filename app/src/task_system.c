/*
 * Copyright (c) 2023 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * @file   : task_system.c
 * @date   : Set 26, 2023
 * @author : Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>
 * @version	v1.0.0
 */

/********************** inclusions *******************************************/
/* Project includes. */
#include "main.h"
#include "string.h"

/* Demo includes. */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes. */
#include "board.h"
#include "app.h"
#include "task_system_attribute.h"
#include "task_system_interface.h"
#include "task_actuator_attribute.h"
#include "task_actuator_interface.h"
// #include "task_temperature.h"
// #include "display.h"

/********************** macros and definitions *******************************/
#define G_TASK_SYS_CNT_INI			0ul
#define G_TASK_SYS_TICK_CNT_INI		0ul

#define DEL_SYST_MIN	 			0ul
#define DEL_SYST_MIN_WAITING_TIME	1ul
#define DEL_SYST_MIN_SPEED          1ul

#define DEL_SYST_INIT_SPEED			10ul
#define DEL_SYST_INIT_PCS			1ul
#define DEL_SYST_INIT_WAITING_TIME	5ul
#define DEL_SYST_INIT_OPTION		1ul

#define DEL_SYST_MAX_PACKS			10ul
#define DEL_SYST_MAX_SPEED			20ul
#define DEL_SYST_MAX_WAITING_TIME	30ul



#define EV_NO_EVENT -1
/********************** internal data declaration ****************************/
task_system_dta_t task_system_dta =
	{DEL_SYST_MIN, 10, DEL_SYST_MIN, DEL_SYST_MIN, DEL_SYST_MIN_WAITING_TIME, DEL_SYST_MIN, ST_SYST_IDLE, ST_SETUP_INIT_MENU, EV_SYST_CTRL_OFF, false};

#define SYSTEM_DTA_QTY	(sizeof(task_system_dta)/sizeof(task_system_dta_t))

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
const char *p_task_system 		= "Task System (System Statechart)";
const char *p_task_system_ 		= "Non-Blocking & Update By Time Code";

/********************** external data declaration ****************************/
uint32_t g_task_system_cnt;
volatile uint32_t g_task_system_tick_cnt;

/********************** external functions definition ************************/
void task_system_init(void *parameters) {
	task_system_dta_t 			*p_task_system_dta;
	task_system_st_t			state;
	task_system_composed_st_t	composed_state;
	task_system_ev_t			event;
	bool 						b_event;

	/* Print out: Task Initialized */
	LOGGER_LOG("  %s is running - %s\r\n", GET_NAME(task_system_init), p_task_system);
	LOGGER_LOG("  %s is a %s\r\n", GET_NAME(task_system), p_task_system_);

	g_task_system_cnt = G_TASK_SYS_CNT_INI;

	/* Print out: Task execution counter */
	LOGGER_LOG("   %s = %lu\r\n", GET_NAME(g_task_system_cnt), g_task_system_cnt);

	init_queue_event_task_system();

	/* Update Task Actuator Configuration & Data Pointer */
	p_task_system_dta = &task_system_dta;

	/* Print out: Task execution FSM */
	state = p_task_system_dta->state;
	LOGGER_LOG("   %s = %lu", GET_NAME(state), (uint32_t)state);

	composed_state = p_task_system_dta->composed_state;
	LOGGER_LOG("   %s = %lu", GET_NAME(composed_state), (uint32_t)composed_state);

	event = p_task_system_dta->event;
	LOGGER_LOG("   %s = %lu", GET_NAME(event), (uint32_t)event);

	b_event = p_task_system_dta->flag;
	LOGGER_LOG("   %s = %s\r\n", GET_NAME(b_event), (b_event ? "true" : "false"));

	g_task_system_tick_cnt = G_TASK_SYS_TICK_CNT_INI;

	//displayInit();
}

void task_system_update(void *parameters) {
	task_system_dta_t *p_task_system_dta;
	bool b_time_update_required = false;

	/* Update Task System Counter */
	g_task_system_cnt++;

	/* Protect shared resource (g_task_system_tick) */
	__asm("CPSID i");	/* disable interrupts*/
    if (G_TASK_SYS_TICK_CNT_INI < g_task_system_tick_cnt) {
    	g_task_system_tick_cnt--;
    	b_time_update_required = true;
    }
    __asm("CPSIE i");	/* enable interrupts*/

    while (b_time_update_required) {
		/* Protect shared resource (g_task_system_tick) */
		__asm("CPSID i");	/* disable interrupts*/
		if (G_TASK_SYS_TICK_CNT_INI < g_task_system_tick_cnt) {
			g_task_system_tick_cnt--;
			b_time_update_required = true;
		}
		else {
			b_time_update_required = false;
		}
		__asm("CPSIE i");	/* enable interrupts*/

    	/* Update Task System Data Pointer */
		p_task_system_dta = &task_system_dta;

		if (true == any_event_task_system()) {
			p_task_system_dta->flag = true;
			p_task_system_dta->event = get_event_task_system();
		}

        /* Fetch events */
        if (true == any_event_task_system()) {
            p_task_system_dta->flag = true;
            p_task_system_dta->event = get_event_task_system();
            LOGGER_LOG("Detected Event: %d, Flag: %s\n",
                       p_task_system_dta->event,
                       p_task_system_dta->flag ? "true" : "false");
        }

		if (p_task_system_dta->flag && p_task_system_dta->event != EV_NO_EVENT) {

			switch (p_task_system_dta->state) {

				case ST_SYST_IDLE:


					if (EV_SYST_CTRL_ON == p_task_system_dta->event) {
						LOGGER_LOG("ENTRE AL SISTEMA DE CONTROL\n");
						p_task_system_dta->state = ST_SYST_CTRL;
						p_task_system_dta->qty_packs = DEL_SYST_MIN;
						p_task_system_dta->speed = DEL_SYST_INIT_SPEED;
						p_task_system_dta->pack_rate = 2;
						p_task_system_dta->waiting_time = DEL_SYST_INIT_WAITING_TIME;
						p_task_system_dta->tick = DEL_SYST_MIN;
					}

					if (EV_SYST_SETUP_ON == p_task_system_dta->event) {
						LOGGER_LOG("ENTRE AL SETUP");
						p_task_system_dta->state = ST_SYST_SETUP;
						p_task_system_dta->qty_packs = DEL_SYST_MIN;
						p_task_system_dta->composed_state = ST_SETUP_INIT_MENU;
						p_task_system_dta->speed = DEL_SYST_INIT_SPEED;
						p_task_system_dta->pack_rate = DEL_SYST_INIT_PCS;
						p_task_system_dta->waiting_time = DEL_SYST_INIT_WAITING_TIME;
						p_task_system_dta->option = DEL_SYST_INIT_OPTION;
						p_task_system_dta->tick = DEL_SYST_MIN;
					}

					break;

				case ST_SYST_CTRL:

					LOGGER_LOG("BIENVENIDO AL SISTEMA DE CONTROL!\n");

//
//					if (p_task_system_dta->qty_packs == DEL_SYST_MIN)
//						put_event_task_actuator(EV_LED_XX_TURN_ON, ID_LED_MIN_SPEED);
//
//					if (p_task_system_dta->qty_packs == DEL_SYST_MAX_PACKS)
//						put_event_task_actuator(EV_LED_XX_TURN_ON, ID_LED_MAX_SPEED);

				    if (EV_SYST_PACK_IN == p_task_system_dta->event) {
				        if (p_task_system_dta->qty_packs < DEL_SYST_MAX_PACKS) {
				            p_task_system_dta->qty_packs++;
				            LOGGER_LOG("AUMENTA LA CANTIDAD DE PACKS A %lu\n", p_task_system_dta->qty_packs);

				            if (p_task_system_dta->speed > DEL_SYST_MIN_SPEED && ((int)p_task_system_dta->qty_packs % (int)p_task_system_dta->pack_rate == 0)) {
				                p_task_system_dta->speed--;
				                LOGGER_LOG("DISMINUYE LA VELOCIDAD A %lu\n", p_task_system_dta->speed);
				                LOGGER_LOG("RESULTADO DE MODULO ENTRE %i y %i es %i", (int)p_task_system_dta->qty_packs, (int)p_task_system_dta->pack_rate, (int)p_task_system_dta->qty_packs % p_task_system_dta->pack_rate);
				            }
				        } else {
				            LOGGER_LOG("SE ALCANZÓ EL LÍMITE DE PACKS (%lu), NO SE PUEDEN AGREGAR MÁS\n", DEL_SYST_MAX_PACKS);
				        }
				        p_task_system_dta->event = EV_NO_EVENT; // Consumir el evento
				    }

					if (EV_SYST_NO_PACKS == p_task_system_dta->event && p_task_system_dta->tick == p_task_system_dta->waiting_time
							&& p_task_system_dta->qty_packs == DEL_SYST_MIN) {
						LOGGER_LOG("NO HAY PACKS Y SE CUMPLIÓ EL TIEMPO DE ESPERA\n");
						put_event_task_system(EV_SYST_CTRL_OFF);
					}

					else if (EV_SYST_NO_PACKS == p_task_system_dta->event && p_task_system_dta->qty_packs == DEL_SYST_MIN) {
						LOGGER_LOG("AUMENTA TIEMPO DE ESPERA SI NO HAY PACKS\n");
						p_task_system_dta->tick++;
					}

				    if (EV_SYST_PACK_OUT == p_task_system_dta->event) {
				        if (p_task_system_dta->qty_packs > DEL_SYST_MIN) {
				            p_task_system_dta->qty_packs--;
				            LOGGER_LOG("DISMINUYE LA CANTIDAD DE PACKS A %lu\n", p_task_system_dta->qty_packs);

				            if (p_task_system_dta->speed < DEL_SYST_MAX_SPEED && ((int)p_task_system_dta->qty_packs % (int)p_task_system_dta->pack_rate == 0)) {
				                p_task_system_dta->speed++; // Aumentar la velocidad
				                LOGGER_LOG("AUMENTA LA VELOCIDAD A %lu\n", p_task_system_dta->speed);
				            }
				        } else {
				            LOGGER_LOG("NO HAY MÁS PACKS PARA ELIMINAR\n");
				        }
				        p_task_system_dta->event = EV_NO_EVENT; // Consumir el evento
				    }

					if (EV_SYST_SETUP_ON == p_task_system_dta->event) {
						LOGGER_LOG("ESTOY EN EL SETUP\n");
						p_task_system_dta->state = ST_SYST_SETUP;
						p_task_system_dta->composed_state = ST_SETUP_INIT_MENU;
						p_task_system_dta->option = 1;
					}

					if (EV_SYST_CTRL_OFF == p_task_system_dta->event) {
						LOGGER_LOG("SE APAGA EL SYST DE CONTROL\n");
						p_task_system_dta->state = ST_SYST_IDLE;
						p_task_system_dta->qty_packs = DEL_SYST_MIN;
						p_task_system_dta->speed = DEL_SYST_MIN;
						p_task_system_dta->pack_rate = DEL_SYST_MIN;
						p_task_system_dta->waiting_time = DEL_SYST_MIN;
						p_task_system_dta->tick = DEL_SYST_MIN;
					}

					break;

				case ST_SYST_SETUP:

				    if (EV_SYST_SETUP_OFF == p_task_system_dta->event) {
				        LOGGER_LOG("SE APAGA EL SISTEMA DE SETUP\n");
				        p_task_system_dta->state = ST_SYST_CTRL;  // Cambia al estado de control
				        p_task_system_dta->composed_state = ST_SETUP_INIT_MENU;  // Reinicia el compuesto
				        p_task_system_dta->option = 1;  // Restablece la opción predeterminada
				        p_task_system_dta->event = EV_NO_EVENT;  // Consumir el evento
				        p_task_system_dta->flag = false;  // Desactivar el flag
				        return;  // Salir del estado actual
				    }

				    switch (p_task_system_dta->composed_state) {


				    	case ST_SETUP_INIT_MENU:
				            LOGGER_LOG("ESTOY EN EL MENU INICIAL DEL SETUP\n");
				            LOGGER_LOG("EVENTO NRO %i\n", p_task_system_dta->event);

				            if (EV_SETUP_NEXT == p_task_system_dta->event && p_task_system_dta->option == 1) {
				                LOGGER_LOG("OPCION 2 INIT MENU\n");
				                p_task_system_dta->option = 2;
				                p_task_system_dta->event = EV_NO_EVENT;  // Consumir evento
				            } else if (EV_SETUP_NEXT == p_task_system_dta->event && p_task_system_dta->option == 2) {
				                LOGGER_LOG("OPCION 1 INIT MENU\n");
				                p_task_system_dta->option = 1;
				                p_task_system_dta->event = EV_NO_EVENT;  // Consumir evento
				            }

				            if (EV_SETUP_ENTER == p_task_system_dta->event && p_task_system_dta->option == 1) {
				                LOGGER_LOG("MENU PACKS LIM\n");
				                p_task_system_dta->composed_state = ST_SETUP_MENU_PACKS_LIM;
				                p_task_system_dta->event = EV_NO_EVENT;  // Consumir evento
				            } else if (EV_SETUP_ENTER == p_task_system_dta->event && p_task_system_dta->option == 2) {
				                LOGGER_LOG("MENU WAITING TIME\n");
				                p_task_system_dta->composed_state = ST_SETUP_MENU_WAITING_TIME;
				                p_task_system_dta->event = EV_NO_EVENT;  // Consumir evento
				            }

				            if (EV_SETUP_ESCAPE == p_task_system_dta->event) {
				                LOGGER_LOG("VUELVO AL MENU INICIAL\n");
				                p_task_system_dta->composed_state = ST_SETUP_INIT_MENU;
				                p_task_system_dta->option = 1;
				                p_task_system_dta->event = EV_NO_EVENT;  // Consumir evento
				            }

				            break;

				        case ST_SETUP_MENU_PACKS_LIM:
				            LOGGER_LOG("ESTOY EN EL MENU DEL PACKS LIM \n");

				            if (EV_SETUP_NEXT == p_task_system_dta->event && p_task_system_dta->pack_rate < DEL_SYST_MAX_PACKS) {
				                p_task_system_dta->pack_rate++;
				                LOGGER_LOG("VARIO EL PACK RATE %lu\n", p_task_system_dta->pack_rate);
				                p_task_system_dta->event = EV_NO_EVENT;  // Consumir evento
				            } else if (EV_SETUP_NEXT == p_task_system_dta->event && p_task_system_dta->pack_rate == DEL_SYST_MAX_PACKS) {
				                p_task_system_dta->pack_rate = 1;
				                LOGGER_LOG("VUELVE A 1\n");
				                p_task_system_dta->event = EV_NO_EVENT;  // Consumir evento
				            }

				            if (EV_SETUP_ESCAPE == p_task_system_dta->event) {
				                LOGGER_LOG("VUELVO AL MENU INICIAL");
				                p_task_system_dta->composed_state = ST_SETUP_INIT_MENU;
				                p_task_system_dta->option = 1;
				                p_task_system_dta->event = EV_NO_EVENT;  // Consumir evento
				            }

				            break;

				        case ST_SETUP_MENU_WAITING_TIME:
				            LOGGER_LOG("ESTOY EN EL MENU DEL WAITING TIME\n");

				            if (EV_SETUP_NEXT == p_task_system_dta->event) {
				                p_task_system_dta->waiting_time++;
				                LOGGER_LOG("VARIO EL WAITING TIME %lu\n", p_task_system_dta->waiting_time);
				            }

				            if (EV_SETUP_NEXT == p_task_system_dta->event && p_task_system_dta->waiting_time == DEL_SYST_MAX_WAITING_TIME) {
				                p_task_system_dta->waiting_time = DEL_SYST_MIN_WAITING_TIME;
				                LOGGER_LOG("VUELVE A 1\n");
				            }

				            if (EV_SETUP_ESCAPE == p_task_system_dta->event) {
				                LOGGER_LOG("VUELVO AL MENU INICIAL")
				                p_task_system_dta->composed_state = ST_SETUP_INIT_MENU;
				                p_task_system_dta->option = 1;
				            }

				            break;
				    }
				    break;

			}
			p_task_system_dta->flag = false;
			p_task_system_dta->event = EV_NO_EVENT;
		}

    }
}

/********************** end of file ******************************************/