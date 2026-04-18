/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   colors.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:57:20 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/18 10:59:55 by abnsila          ###   ########.fr       */
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
#define BG_RED  "\x1B[41m"
#define BG_GRN  "\x1B[42m"
#define BG_YEL  "\x1B[43m"
#define BG_BLU  "\x1B[44m"
#define BG_MAG  "\x1B[45m"
#define BG_CYN  "\x1B[46m"
#define BG_WHT  "\x1B[47m"
#define BG_BLK "\x1B[1;30m"
#define BG_ORG "\x1B[1;38;5;208m"

/* BOLD */
#define B_RED "\x1B[1;31m"
#define B_GRN "\x1B[1;32m"
#define B_YEL "\x1B[1;33m"
#define B_BLU "\x1B[1;34m"
#define B_MAG "\x1B[1;35m"
#define B_CYN "\x1B[1;36m"
#define B_WHT "\x1B[1;37m"
#define B_ORG "\x1B[1;38;5;208;1m"

/* ITALIC */
#define I_RED "\x1B[3;31m"
#define I_GRN "\x1B[3;32m"
#define I_YEL "\x1B[3;33m"
#define I_BLU "\x1B[3;34m"
#define I_MAG "\x1B[3;35m"
#define I_CYN "\x1B[3;36m"
#define I_WHT "\x1B[3;37m"
#define I_ORG "\x1B[38;5;208;3m"

/* BOLD FOREGROUND */
#define BOLD "\x1B[1m"
#define UNBOLD "\x1B[22m"
