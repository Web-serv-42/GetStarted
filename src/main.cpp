/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:59:03 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/18 11:26:45 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Core/Log.hpp"
#include "Timer.hpp"

int main(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;

	Timer::Init();

	DEBUG_LOG("Hello World");
	TRACE_LOG("Hello World");
	WARNING_LOG("Hello World");
	CRITICAL_LOG("Hello World");
	ERROR_LOG("Hello World");
	INFO_LOG("Hello World");
	SUCCESS_LOG("Hello World");

	return 0;
}
