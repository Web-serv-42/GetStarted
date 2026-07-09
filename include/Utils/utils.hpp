/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/20 13:03:01 by abnsila           #+#    #+#             */
/*   Updated: 2026/05/20 13:09:27 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Core/Timer.hpp"
#include <string>
#include <sstream>

std::string	GenerateTmpFileName(std::string contex);
std::string toString(int port);