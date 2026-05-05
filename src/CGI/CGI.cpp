/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/02 21:24:00 by abnsila           #+#    #+#             */
/*   Updated: 2026/05/04 14:18:12 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI/CGI.hpp"
#include "Core/Timer.hpp"

CGI::CGI(/* args */)
{
}

CGI::CGI(std::string path) : m_ScriptPath(path)
{
	
}

void	CGI::Run()
{
	TimerBenchmark	timer = TimerBenchmark();

	// 
}

CGI::~CGI()
{
}
