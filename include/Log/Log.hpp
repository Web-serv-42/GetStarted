/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:08:12 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/17 18:58:39 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <string>

// Absctract class + colors

class Log
{
	private:
		std::string	name;
	public:
		Log();
		~Log();
		static void	trace(std::string	msg);
		static void	info(std::string	msg);
		static void	warn(std::string	msg);
		static void	error(std::string	msg);
		static void	success(std::string	msg);
		static void	critical(std::string	msg);
};
