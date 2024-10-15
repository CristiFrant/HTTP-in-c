#ifndef _REQUESTS_
#define _REQUESTS_

#include <string>

// computes and returns a GET request string (query_params and cookies can be 
// set to NULL if not needed)
char *compute_get_request(const char *host, const char *url, const char *query_params,
							const char *cookies, const char *JWT);

// computes and returns a POST request string (cookies can be NULL if not needed)
char *compute_post_request(const char *host, const char *url, const char* content_type,
							const char *body_data, const char* cookies, const char *JWT);

// computes and returns a DELETE request string (query_params and cookies can be
// set to NULL if not needed)
char *compute_delete_request(const char *host, const char *url, const char *query_params,
                            const char *cookies, const char* JWT);

#endif
