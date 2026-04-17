/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   colors.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:57:20 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/17 19:06:43 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

/* FOREGROUND */
#define RST  "\x1B[0m"
#define RED  "\x1B[31m"
#define GRN  "\x1B[32m"
#define YEL  "\x1B[33m"
#define BLU  "\x1B[34m"
#define MAG  "\x1B[35m"
#define CYN  "\033[0;96m"
#define WHT  "\x1B[37m"
#define BLK  "\x1B[30m"
#define ORG  "\x1B[38;5;208m"

/* BACKGROUND */
#define B_RED  "\x1B[41m"
#define B_GRN  "\x1B[42m"
#define B_YEL  "\x1B[43m"
#define B_BLU  "\x1B[44m"
#define B_MAG  "\x1B[45m"
#define B_CYN  "\x1B[46m"
#define B_WHT  "\x1B[47m"
#define B_BLK "\x1B[1;30m"
#define B_ORG "\x1B[1;38;5;208m"


/* BOLD FOREGROUND */
#define BOLD "\x1B[1m"
#define UNBOLD "\x1B[22m"
