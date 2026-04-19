/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 13:00:40 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/19 15:47:15 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Core/Log.hpp"

class Webserv
{
	private:
		bool	m_IsRunning;
	public:
		Webserv(/* args */);
		~Webserv();

		bool	Init();
		void	Run();
		void	Shutdown();
};
