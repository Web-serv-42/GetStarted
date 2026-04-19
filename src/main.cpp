/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:59:03 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/19 15:52:51 by abnsila          ###   ########.fr       */
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
