/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpStatus.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 10:18:09 by abnsila           #+#    #+#             */
/*   Updated: 2026/07/13 10:24:37 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

enum HttpStatusCode
{
    DROP_CONNECTION            = -1,
    NORMAL                     = 0,
    // 2xx Success
    HTTP_OK                    = 200,
    HTTP_CREATED               = 201,
    HTTP_NO_CONTENT            = 204,

    // 3xx Redirection
    HTTP_MOVED_PERMANENTLY     = 301,
    HTTP_FOUND                 = 302,

    // 4xx Client Errors
    HTTP_BAD_REQUEST           = 400,
    HTTP_FORBIDDEN             = 403,
    HTTP_NOT_FOUND             = 404,
    HTTP_METHOD_NOT_ALLOWED    = 405,
    HTTP_REQUEST_TIMEOUT       = 408,
    HTTP_LENGTH_REQUIRED       = 411,
    HTTP_PAYLOAD_TOO_LARGE     = 413,
    HTTP_UNSUPPORTED_MEDIA_TYPE = 415,

    // 5xx Server Errors
    HTTP_INTERNAL_SERVER_ERROR = 500,
    HTTP_NOT_IMPLEMENTED       = 501,
    HTTP_BAD_GATEWAY           = 502,
    HTTP_GATEWAY_TIMEOUT       = 504,
    HTTP_VERSION_NOT_SUPPORTED = 505
};

// Helper function to get the RFC reason phrase string
std::string GetHttpStatusReason(HttpStatusCode statusCode);
