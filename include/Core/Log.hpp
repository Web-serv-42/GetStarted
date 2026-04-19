/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:08:12 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/19 15:40:19 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <string>
#include "Core/colors.h"
#include "Timer.hpp"

enum	LogLevel
{
	DEBUG,
	TRACE,
	WARNING,
	CRITICAL,
	ERROR,
	INFO,
	SUCCESS,
};

class Log
{
	public:
		~Log();
		static void	Output(LogLevel level, const std::string& file, int line, const std::string& msg);

	private:
		Log();
};

#define DEBUG_LOG(msg)		Log::Output(DEBUG, __FILE__, __LINE__, msg)
#define TRACE_LOG(msg)		Log::Output(TRACE, __FILE__, __LINE__, msg)
#define WARNING_LOG(msg)	Log::Output(WARNING, __FILE__, __LINE__, msg)
#define CRITICAL_LOG(msg)	Log::Output(CRITICAL, __FILE__, __LINE__, msg)
#define ERROR_LOG(msg)		Log::Output(ERROR, __FILE__, __LINE__, msg)
#define INFO_LOG(msg)		Log::Output(INFO, __FILE__, __LINE__, msg)
#define SUCCESS_LOG(msg)	Log::Output(SUCCESS, __FILE__, __LINE__, msg)
