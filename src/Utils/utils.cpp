/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/20 12:37:03 by abnsila           #+#    #+#             */
/*   Updated: 2026/05/20 13:03:36 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils/utils.hpp"

std::string	GenerateTmpFileName(std::string contex)
{
	char	buf[100];
	struct tm	tm = Timer::GetTime();
	
	std::strftime(buf, sizeof(buf), "_%s", &tm);
	return ("/tmp/" + contex + std::string(buf) + ".tmp");
}
