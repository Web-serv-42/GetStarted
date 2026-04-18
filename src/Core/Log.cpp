/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:09:40 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/18 11:26:33 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Core/Log.hpp"
#include "Core/colors.h"
#include "Timer.hpp"

Log::Log() {}

Log::~Log() {}

void	Log::Output(LogLevel level, const std::string& file, int line, const std::string& msg)
{
	std::string	timestamp = Timer::GetLogTime();
	std::string	color;
	std::string	label;
	std::ostream* os = &std::cout;

	switch (level)
	{
		case DEBUG: color = WHT; label = "[DEBUG]"; break;
		case TRACE: color = ORG; label = "[TRACE]"; break;
		case INFO: color = CYN; label = "[INFO]"; break;
		case WARNING: color = YEL; label = "[WARNING]"; break;
		case CRITICAL: color = B_RED; label = "[CRITICAL]"; break;
		case ERROR: color = RED; label = "[ERROR]"; break;
		case SUCCESS: color = GRN; label = "[SUCCESS]"; break;
	}

	// 3. Print the formatted line
	// Format: [Timestamp] [LEVEL] [File:Line] Message
	*os << WHT << timestamp << " " << RST
		<< color << label << " " << RST
		<< I_BLU  << "[" << file << ":" << line << "] " << RST
		<< msg << std::endl;
}
