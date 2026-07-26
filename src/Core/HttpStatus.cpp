/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpStatus.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 10:18:07 by abnsila           #+#    #+#             */
/*   Updated: 2026/07/13 10:18:08 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Core/HttpStatus.hpp"

std::string GetHttpStatusReason(HttpStatusCode statusCode) {
    switch (statusCode) {
        case HTTP_OK:                     return "OK";
        case HTTP_CREATED:                return "Created";
        case HTTP_NO_CONTENT:             return "No Content";
        
        case HTTP_MOVED_PERMANENTLY:      return "Moved Permanently";
        case HTTP_FOUND:                  return "Found";

        case HTTP_BAD_REQUEST:            return "Bad Request";
        case HTTP_FORBIDDEN:              return "Forbidden";
        case HTTP_NOT_FOUND:              return "Not Found";
        case HTTP_METHOD_NOT_ALLOWED:     return "Method Not Allowed";
        case HTTP_REQUEST_TIMEOUT:        return "Request Timeout";
        case HTTP_LENGTH_REQUIRED:        return "Length Required";
        case HTTP_PAYLOAD_TOO_LARGE:      return "Payload Too Large";
        case HTTP_UNSUPPORTED_MEDIA_TYPE: return "Unsupported Media Type";

        case HTTP_INTERNAL_SERVER_ERROR: return "Internal Server Error";
        case HTTP_NOT_IMPLEMENTED:       return "Not Implemented";
        case HTTP_BAD_GATEWAY:           return "Bad Gateway";
        case HTTP_GATEWAY_TIMEOUT:       return "Gateway Timeout";
        case HTTP_VERSION_NOT_SUPPORTED: return "HTTP Version Not Supported";
        
        default:                         return "Unknown Status Code";
    }
}
