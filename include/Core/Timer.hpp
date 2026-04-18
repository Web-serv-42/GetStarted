/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Timer.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 19:36:29 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/18 10:17:05 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sys/time.h>
#include <ctime>
#include <iostream>

class Timer
{
	protected:
		Timer();

	private:
		static struct timeval	s_ServerStart;
		struct timeval			m_LocalStart;
		
	public:
		virtual ~Timer() = 0;

		static void	Init();
		static std::string	GetLogTime();
		static double		GetServerUptime();
		
		void			Reset();
		double			Elapsed() const;
		double			ElapsedMillis() const;
};

class TimerBenchmark : public Timer
{
	public:
		TimerBenchmark() : Timer() {};
		~TimerBenchmark();
};
