/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/02 21:23:42 by abnsila           #+#    #+#             */
/*   Updated: 2026/05/05 11:42:53 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>

#include <sys/types.h>

class CGI
{
	private:
		std::string					m_ScriptPath;
		// pid_t						m_Pid;
		// int							m_Pipe[2];
		// std::vector<std::string>	m_EnvVars;
	public:
		CGI(/* args */);
		CGI(std::string path);
		~CGI();

		void	Run();
};

