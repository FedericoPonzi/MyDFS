/*
 * 
 * Copyright 2015 Federico Ponzi <federicoponzi@federicos-ubuntu>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * CHANGELOG:
 * 	11.07.2015: Creazione del file.
 * 
 */
/**
 * @file Bye.c
 * @author Federico Ponzi
 * @date 11.07.2015
 *
 * @brief Gestisce il comando BYE
 *
 * Quando il client invia un comando BYE, questo modulo lo gestisce chiudendo la socket( ma non uscendo dal programma).
 */
#include <stdio.h>
#include "inc/BYE.h"
#include <unistd.h>
#include "inc/Utils.h"

/**
 * @brief Gestisce il comando Bye
 * @todo Dovrebbe cercare e rimuovere i file aperti dal client.
 */
void handleByeCommand(int socket)
{
	close(socket);
}
