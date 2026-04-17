/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:59:03 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/17 19:00:30 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Log/Log.hpp"

int main(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;

	Log::info("Hello World");
	Log::critical("Hello World");
	Log::error("Hello World");
	Log::success("Hello World");
	Log::trace("Hello World");
	Log::warn("Hello World");
	return 0;
}
