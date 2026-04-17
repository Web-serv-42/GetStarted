/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:09:40 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/17 19:04:10 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Log/Log.hpp"
#include "Log/colors.h"

Log::Log()
{
}

Log::~Log()
{
}

void	Log::trace(std::string msg)
{
	std::cout << BLU << msg << RST  << std::endl;
}

void	Log::info(std::string msg)
{
	std::cout << CYN << msg << RST << std::endl;
}

void	Log::warn(std::string msg)
{
	std::cout << YEL << msg << RST << std::endl;
}

void	Log::error(std::string msg)
{
	std::cout << RED << msg << RST << std::endl;
}

void	Log::success(std::string	msg)
{
	std::cout << GRN << msg << RST << std::endl;
}

void	Log::critical(std::string msg)
{
	std::cout << BOLD << RED << msg << RST << std::endl;
}
