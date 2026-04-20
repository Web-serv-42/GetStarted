/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:59:03 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/20 15:27:20 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Core/Log.hpp"
#include "Core/Timer.hpp"
#include "Server/Webserv.hpp"

int main(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;

	Webserv	engine;

	engine.Init();
	engine.Run();
	engine.Shutdown();

	return 0;
}



// now i understand that my Webserv.Init():
// create and setup servers and attached to their ports, and start listening (queue pending)

// thanks to TcpServer.Setup() that target the single server and setup it, and i store them inside my Webserv, so i can move to Webser.Run() (main loop):
// the core here understanding the epoll (epoll_create, epoll_ctl,
// epoll_wait), accept, now the servers wait in the listen queue right ??
// what s next what is the role of epoll ??
// i know this is belongs to Multiplixer class
// it containe :
// setup epoll (but what is epoll)
// add socket to epoll ??
// setup events
// loop and wait for new connection, data recived, data to be sended
// chuncked data ?
