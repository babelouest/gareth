/**
 *
 * Gareth messenger service
 *
 * Get messages from a REST Webservice
 * Send messages or digested data when previously parametered filters are triggered
 * Send protocols available: http, smtp
 *
 * Unit tests
 *
 * Copyright 2015-2016 Nicolas Mora <mail@babelouest.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU GENERAL PUBLIC LICENSE
 * License as published by the Free Software Foundation;
 * version 3 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU GENERAL PUBLIC LICENSE for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <string.h>
#include <jansson.h>
#include <ulfius.h>
#include <orcania.h>

#define SERVER_URL_PREFIX "http://localhost:2474/gareth"

#define AUTH_SERVER_URI "http://localhost:4593/glewlwyd"
#define USER_LOGIN "user1"
#define USER_PASSWORD "MyUser1Password!"
#define USER_SCOPE_LIST "angharad"

char * token = NULL;

/**
 * decode a u_map into a string
 */
char * print_map(const struct _u_map * map) {
  char * line, * to_return = NULL;
  const char **keys;
  int len, i;
  if (map != NULL) {
    keys = u_map_enum_keys(map);
    for (i=0; keys[i] != NULL; i++) {
      len = snprintf(NULL, 0, "key is %s, value is %s\n", keys[i], u_map_get(map, keys[i]));
      line = malloc((len+1)*sizeof(char));
      snprintf(line, (len+1), "key is %s, value is %s\n", keys[i], u_map_get(map, keys[i]));
      if (to_return != NULL) {
        len = strlen(to_return) + strlen(line) + 1;
        to_return = realloc(to_return, (len+1)*sizeof(char));
      } else {
        to_return = malloc((strlen(line) + 1)*sizeof(char));
        to_return[0] = 0;
      }
      strcat(to_return, line);
      free(line);
    }
    return to_return;
  } else {
    return NULL;
  }
}

/**
 * Developper-friendly response print
 */
void print_response(struct _u_response * response) {
  if (response != NULL) {
    printf("Status: %ld\n\n", response->status);
    printf("Body:\n%.*s\n\n", (int)response->binary_body_length, (char *)response->binary_body);
  }
}

int test_request_status(struct _u_request * req, long int expected_status, json_t * expected_contains) {
  int res, to_return = 0;
  struct _u_response response;
  
  ulfius_init_response(&response);
  res = ulfius_send_http_request(req, &response);
  if (res == U_OK) {
    json_t * json_body = ulfius_get_json_body_response(&response, NULL);
    if (response.status != expected_status) {
      printf("##########################\nError status (%s %s %ld)\n", req->http_verb, req->http_url, expected_status);
      print_response(&response);
      printf("##########################\n\n");
    } else if (expected_contains != NULL && (json_body == NULL || json_search(json_body, expected_contains) == NULL)) {
      char * dump_expected = json_dumps(expected_contains, JSON_ENCODE_ANY), * dump_response = json_dumps(json_body, JSON_ENCODE_ANY);
      printf("##########################\nError json (%s %s)\n", req->http_verb, req->http_url);
      printf("Expected result in response:\n%s\nWhile response is:\n%s\n", dump_expected, dump_response);
      printf("##########################\n\n");
      free(dump_expected);
      free(dump_response);
    } else {
      printf("Success (%s %s %ld)\n\n", req->http_verb, req->http_url, expected_status);
      to_return = 1;
    }
    json_decref(json_body);
  } else {
    printf("Error in http request: %d\n", res);
  }
  ulfius_clean_response(&response);
  return to_return;
}

void run_simple_test(const char * method, const char * url, json_t * request_body, int expected_status, json_t * expected_body) {
  struct _u_request request;
  ulfius_init_request(&request);
  request.http_verb = strdup(method);
  request.http_url = strdup(url);
  if (token != NULL) {
    u_map_put(request.map_header, "Authorization", token);
  }
  ulfius_set_json_body_request(&request, json_copy(request_body));
  
  test_request_status(&request, expected_status, expected_body);
  
  ulfius_clean_request(&request);
}

void run_smtp_alert_tests() {
  json_t * smtp_post_alert_valid = json_loads("{\
  \"name\": \"smtp1\",\
  \"host\": \"localhost\",\
  \"from\": \"moi\",\
  \"to\": \"lui\",\
  \"subject\": \"message de moi\",\
  \"body\": \"a toi et eux\"\
}", JSON_DECODE_ANY, NULL);
  json_t * smtp_post_alert_invalid = json_loads("{\
  \"name\": \"smtp2\",\
  \"host\": \"localhost\",\
  \"port\": \"e\",\
  \"from\": \"moi\",\
  \"to\": \"lui\",\
  \"subject\": \"message de moi\",\
  \"body\": \"a toi et eux\"\
}", JSON_DECODE_ANY, NULL);
  json_t * smtp_put_alert_valid = json_loads("{\
  \"name\": \"smtp1\",\
  \"host\": \"localhost2\",\
  \"tls\": true,\
  \"check_ca\": false,\
  \"user\": \"john\",\
  \"password\": \"therupper\",\
  \"from\": \"moi\",\
  \"to\": \"lui\",\
  \"subject\": \"message de lui\",\
  \"body\": \"a moi et pas a eux\"\
}", JSON_DECODE_ANY, NULL);
  json_t * smtp_put_alert_invalid = json_loads("{\
  \"name\": \"smtp1\",\
  \"host\": \"localhost\",\
  \"port\": \"e\",\
  \"from\": \"moi\",\
  \"to\": \"lui\",\
  \"subject\": \"message de moi\",\
  \"body\": \"a toi et eux\"\
}", JSON_DECODE_ANY, NULL);
json_t * smtp_alert_check = json_loads("{\
  \"check_ca\": false,\
  \"name\": \"smtp1\",\
  \"to\": \"lui\",\
  \"from\": \"moi\",\
  \"tls\": false,\
  \"subject\": \"message de moi\",\
  \"host\": \"localhost\",\
  \"port\": 0,\
  \"body\": \"a toi et eux\"\
}", JSON_DECODE_ANY, NULL);
  
  run_simple_test("GET", SERVER_URL_PREFIX "/alert/", NULL, 200, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/alert/smtp/", smtp_post_alert_valid, 200, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/alert/smtp/", smtp_post_alert_invalid, 400, NULL);
  run_simple_test("GET", SERVER_URL_PREFIX "/alert/smtp/smtp1", NULL, 200, smtp_alert_check);
  run_simple_test("GET", SERVER_URL_PREFIX "/alert/smtp/smtp2", NULL, 404, NULL);
  run_simple_test("PUT", SERVER_URL_PREFIX "/alert/smtp/smtp1", smtp_put_alert_valid, 200, NULL);
  run_simple_test("PUT", SERVER_URL_PREFIX "/alert/smtp/smtp1", smtp_put_alert_invalid, 400, NULL);
  run_simple_test("DELETE", SERVER_URL_PREFIX "/alert/smtp/smtp2", NULL, 404, NULL);
  run_simple_test("DELETE", SERVER_URL_PREFIX "/alert/smtp/smtp1", NULL, 200, NULL);
  
  json_decref(smtp_post_alert_valid);
  json_decref(smtp_post_alert_invalid);
  json_decref(smtp_put_alert_valid);
  json_decref(smtp_put_alert_invalid);
  json_decref(smtp_alert_check);
}

void run_http_alert_tests() {
  json_t * http_post_alert_valid = json_loads("{\
  \"name\": \"http1\",\
  \"method\": \"GET\",\
  \"url\": \"http://localhost/\",\
  \"body\": \"plop\",\
  \"http_headers\": [\
    {\"key\": \"header1\", \"value\": \"value1\"},\
    {\"key\": \"header2\", \"value\": \"value2\"}\
  ]\
}", JSON_DECODE_ANY, NULL);
  json_t * http_post_alert_invalid = json_loads("{\
  \"name\": \"http2\",\
  \"method\": 22,\
  \"url\": \"http://localhost/\",\
  \"body\": \"plop\"\
}", JSON_DECODE_ANY, NULL);
  json_t * http_put_alert_valid = json_loads("{\
  \"method\": \"GET\",\
  \"url\": \"http://localhost/\",\
  \"body\": \"plop\",\
  \"http_headers\": [\
    {\"key\": \"header1\", \"value\": \"value1\"},\
    {\"key\": \"header2\", \"value\": \"value2\"}\
  ]\
}", JSON_DECODE_ANY, NULL);
  json_t * http_put_alert_invalid = json_loads("{\
  \"method\": \"GET\",\
  \"url\": \"http://localhost/\",\
  \"body\": \"plop\",\
  \"http_headers\": [\
    {\"key\": \"header1\", \"valuee\": \"value1\"},\
    {\"key\": \"header2\", \"value\": \"value2\"}\
  ]\
}", JSON_DECODE_ANY, NULL);
  
  run_simple_test("GET", SERVER_URL_PREFIX "/alert/", NULL, 200, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/alert/http/", http_post_alert_valid, 200, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/alert/http/", http_post_alert_invalid, 400, NULL);
  run_simple_test("GET", SERVER_URL_PREFIX "/alert/http/http1", NULL, 200, http_post_alert_valid);
  run_simple_test("GET", SERVER_URL_PREFIX "/alert/http/http2", NULL, 404, NULL);
  run_simple_test("PUT", SERVER_URL_PREFIX "/alert/http/http1", http_put_alert_valid, 200, NULL);
  run_simple_test("PUT", SERVER_URL_PREFIX "/alert/http/http1", http_put_alert_invalid, 400, NULL);
  run_simple_test("DELETE", SERVER_URL_PREFIX "/alert/http/http2", NULL, 404, NULL);
  run_simple_test("DELETE", SERVER_URL_PREFIX "/alert/http/http1", NULL, 200, NULL);
  
  json_decref(http_post_alert_valid);
  json_decref(http_post_alert_invalid);
  json_decref(http_put_alert_valid);
  json_decref(http_put_alert_invalid);
}

void run_filter_tests() {
  json_t * filter_post_valid = json_loads("{\
  \"name\": \"filter1\",\
  \"description\": \"valid test\",\
  \"filter_clauses\": [\
    {\"element\": \"date\", \"operator\": \">=\", \"value\": 123456},\
    {\"element\": \"priority\", \"operator\": \">\", \"value\": 1},\
    {\"element\": \"source\", \"operator\": \"=\", \"value\": \"benoic\"},\
    {\"element\": \"message\", \"operator\": \"contains\", \"value\": \"switch\"},\
    {\"element\": \"tag\", \"operator\": \"contains\", \"value\": \"plop\"}\
  ],\
  \"filter_alerts\": {\"smtp\": [\"smtptest1\"], \"http\": [\"httptest1\"]}\
}", JSON_DECODE_ANY, NULL);
  json_t * filter_post_invalid = json_loads("{\
  \"name\": \"filter2\",\
  \"description\": \"invalid test\",\
  \"filter_clauses\": [\
    {\"element\": \"date\", \"operator\": \"ty\", \"value\": 123456},\
    {\"element\": \"priority\", \"operator\": \">\", \"value\": 1},\
    {\"element\": \"source\", \"operator\": \"=\", \"value\": \"benoic\"},\
    {\"element\": \"message\", \"operator\": \"contains\", \"value\": \"switch\"},\
    {\"element\": \"tag\", \"operator\": \"contains\", \"value\": \"plop\"}\
  \"filter_alerts\": {\"sdfgmtp\": [\"smtp1\"]}\
  ]\
}", JSON_DECODE_ANY, NULL);
  json_t * filter_put_valid = json_loads("{\
  \"name\": \"filter1\",\
  \"description\": \"valid test\",\
  \"filter_clauses\": [\
    {\"element\": \"priority\", \"operator\": \">\", \"value\": 1},\
    {\"element\": \"message\", \"operator\": \"contains\", \"value\": \"switch\"}\
  ],\
  \"filter_alerts\": {\"smtp\": [\"smtptest1\"], \"http\": [\"httptest1\"]}\
}", JSON_DECODE_ANY, NULL);
  json_t * filter_put_invalid = json_loads("{\
  \"name\": \"filter1\",\
  \"description\": \"invalid test\",\
  \"filter_clauses\": [\
    {\"element\": \"date\", \"operator\": \">=\", \"value\": 123456},\
    {\"element\": \"priority\", \"operator\": \">\", \"value\": \"lklk\"},\
    {\"element\": \"tag\", \"operator\": \"contains\", \"value\": \"plop\"}\
  ]\
}", JSON_DECODE_ANY, NULL);
  json_t * smtp_post_alert_valid = json_loads("{\
  \"name\": \"smtptest1\",\
  \"host\": \"localhost\",\
  \"from\": \"moi\",\
  \"to\": \"lui\",\
  \"subject\": \"message de moi\",\
  \"body\": \"a toi et eux\"\
}", JSON_DECODE_ANY, NULL);
  json_t * http_post_alert_valid = json_loads("{\
  \"name\": \"httptest1\",\
  \"method\": \"GET\",\
  \"url\": \"http://localhost/\",\
  \"body\": \"plop\",\
  \"http_headers\": [\
    {\"key\": \"header1\", \"value\": \"value1\"},\
    {\"key\": \"header2\", \"value\": \"value2\"}\
  ]\
}", JSON_DECODE_ANY, NULL);
  
  run_simple_test("GET", SERVER_URL_PREFIX "/filter/", NULL, 200, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/alert/http/", http_post_alert_valid, 200, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/alert/smtp/", smtp_post_alert_valid, 200, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/filter/", filter_post_valid, 200, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/filter/", filter_post_invalid, 400, NULL);
  run_simple_test("GET", SERVER_URL_PREFIX "/filter/filter1", NULL, 200, filter_post_valid);
  run_simple_test("GET", SERVER_URL_PREFIX "/filter/filter2", NULL, 404, NULL);
  run_simple_test("PUT", SERVER_URL_PREFIX "/filter/filter1", filter_put_valid, 200, NULL);
  run_simple_test("PUT", SERVER_URL_PREFIX "/filter/filter1", filter_put_invalid, 400, NULL);
  run_simple_test("DELETE", SERVER_URL_PREFIX "/filter/filter2", NULL, 404, NULL);
  run_simple_test("DELETE", SERVER_URL_PREFIX "/filter/filter1", NULL, 200, NULL);
  run_simple_test("DELETE", SERVER_URL_PREFIX "/alert/http/httptest1", NULL, 200, NULL);
  run_simple_test("DELETE", SERVER_URL_PREFIX "/alert/smtp/smtptest1", NULL, 200, NULL);
  
  json_decref(http_post_alert_valid);
  json_decref(smtp_post_alert_valid);
  json_decref(filter_post_valid);
  json_decref(filter_post_invalid);
  json_decref(filter_put_valid);
  json_decref(filter_put_invalid);
}

void run_add_message_tests() {
  json_t * message_post_valid1 = json_loads("{\
  \"priority\": \"NONE\",\
  \"source\": \"Socrates\",\
  \"text\": \"δαίμων\",\
  \"tags\": [\"tag1\", \"tag2\", \"tag3\"]\
}", JSON_DECODE_ANY, NULL);
  json_t * message_post_valid2 = json_loads("{\
  \"priority\": \"LOW\",\
  \"source\": \"Socrates\",\
  \"text\": \"ἔλεγχος\",\
  \"tags\": [\"tag1\"]\
}", JSON_DECODE_ANY, NULL);
  json_t * message_post_valid3 = json_loads("{\
  \"priority\": \"MEDIUM\",\
  \"source\": \"Plato\",\
  \"text\": \"διάνοια diánoia\",\
  \"tags\": [\"tag3\"]\
}", JSON_DECODE_ANY, NULL);
  json_t * message_post_valid4 = json_loads("{\
  \"priority\": \"HIGH\",\
  \"source\": \"Aristotle\",\
  \"text\": \"νοῦς τῆς διατριβῆς\",\
  \"tags\": [\"tag2\", \"tag3\"]\
}", JSON_DECODE_ANY, NULL);
  json_t * message_post_valid5 = json_loads("{\
  \"priority\": \"CRITICAL\",\
  \"source\": \"Democritus\",\
  \"text\": \"η ιδέα ά-τομος\",\
  \"tags\": [\"tag1\", \"tag3\"]\
}", JSON_DECODE_ANY, NULL);
  json_t * message_post_invalid1 = json_loads("{\
  \"priority\": \"LOWW\",\
  \"source\": \"Unit test\",\
  \"text\": \"This is a invalid test!\",\
  \"tags\": [\"tag1\", \"tag2\", \"tag3\"]\
}", JSON_DECODE_ANY, NULL);
  json_t * message_post_invalid2 = json_loads("{\
  \"priority\": \"LOW\",\
  \"source\": \"Unit test\",\
  \"text\": \"This is a invalid too long testdddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddssssssssThis is a invalid testdddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddssssssss!\",\
  \"tags\": [\"tag1\", \"tag2\", \"tag3\"]\
}", JSON_DECODE_ANY, NULL);
  json_t * message_post_invalid3 = json_loads("{\
  \"priority\": \"LOW\",\
  \"source\": \"Unit test LONG SOURCE eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\",\
  \"text\": \"This is a invalid test!\",\
  \"tags\": [\"tag1\", \"tag2\", \"tag3\"]\
}", JSON_DECODE_ANY, NULL);
  json_t * message_post_invalid4 = json_loads("{\
  \"priority\": \"LOW\",\
  \"source\": \"Unit test\",\
  \"text\": \"This is a invalid test!\",\
  \"tags\": [1, \"tag2\", \"tag3\"]\
}", JSON_DECODE_ANY, NULL);
  json_t * message_post_invalid5 = json_loads("{\
  \"priority\": \"LOW\",\
  \"source\": \"Unit test\",\
  \"text\": \"This is a invalid test!\",\
  \"tags\": [\"tag2 invalideeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\", \"tag3\"]\
}", JSON_DECODE_ANY, NULL);
  
  run_simple_test("POST", SERVER_URL_PREFIX "/message/", message_post_valid1, 200, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/message/", message_post_valid2, 200, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/message/", message_post_valid3, 200, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/message/", message_post_valid4, 200, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/message/", message_post_valid5, 200, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/message/", message_post_invalid1, 400, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/message/", message_post_invalid2, 400, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/message/", message_post_invalid3, 400, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/message/", message_post_invalid4, 400, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/message/", message_post_invalid5, 400, NULL);
  run_simple_test("GET", SERVER_URL_PREFIX "/message/", NULL, 200, NULL);
  
  json_decref(message_post_valid1);
  json_decref(message_post_valid2);
  json_decref(message_post_valid3);
  json_decref(message_post_valid4);
  json_decref(message_post_valid5);
  json_decref(message_post_invalid1);
  json_decref(message_post_invalid2);
  json_decref(message_post_invalid3);
  json_decref(message_post_invalid4);
  json_decref(message_post_invalid5);
}

void run_get_message_tests() {
  json_t * filter_post_valid1 = json_loads("{\
  \"name\": \"filter1\",\
  \"description\": \"valid test\",\
  \"filter_clauses\": [\
    {\"element\": \"priority\", \"operator\": \"=\", \"value\": 2}\
  ]\
}", JSON_DECODE_ANY, NULL);
  json_t * filter_post_valid2 = json_loads("{\
  \"name\": \"filter2\",\
  \"description\": \"valid test\",\
  \"filter_clauses\": [\
    {\"element\": \"priority\", \"operator\": \">\", \"value\": 2}\
  ]\
}", JSON_DECODE_ANY, NULL);
  json_t * filter_post_valid3 = json_loads("{\
  \"name\": \"filter3\",\
  \"description\": \"valid test\",\
  \"filter_clauses\": [\
    {\"element\": \"tag\", \"operator\": \"contains\", \"value\": \"tag3\"}\
  ]\
}", JSON_DECODE_ANY, NULL);
  json_t * filter_post_valid4 = json_loads("{\
  \"name\": \"filter4\",\
  \"description\": \"valid test\",\
  \"filter_clauses\": [\
    {\"element\": \"date\", \"operator\": \">=\", \"value\": 1000}\
  ]\
}", JSON_DECODE_ANY, NULL);

  run_simple_test("POST", SERVER_URL_PREFIX "/filter/", filter_post_valid1, 200, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/filter/", filter_post_valid2, 200, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/filter/", filter_post_valid3, 200, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/filter/", filter_post_valid4, 200, NULL);
  run_simple_test("GET", SERVER_URL_PREFIX "/message/", NULL, 200, NULL);
  run_simple_test("GET", SERVER_URL_PREFIX "/message/filter1", NULL, 200, NULL);
  run_simple_test("GET", SERVER_URL_PREFIX "/message/filter2", NULL, 200, NULL);
  run_simple_test("GET", SERVER_URL_PREFIX "/message/filter3", NULL, 200, NULL);
  run_simple_test("GET", SERVER_URL_PREFIX "/message/filter4", NULL, 200, NULL);
  run_simple_test("DELETE", SERVER_URL_PREFIX "/filter/filter1", NULL, 200, NULL);
  run_simple_test("DELETE", SERVER_URL_PREFIX "/filter/filter2", NULL, 200, NULL);
  run_simple_test("DELETE", SERVER_URL_PREFIX "/filter/filter3", NULL, 200, NULL);
  run_simple_test("DELETE", SERVER_URL_PREFIX "/filter/filter4", NULL, 200, NULL);
  
  json_decref(filter_post_valid1);
  json_decref(filter_post_valid2);
  json_decref(filter_post_valid3);
  json_decref(filter_post_valid4);
}

void run_message_trigger_alert_tests() {
  json_t * smtp_post_alert_valid = json_loads("{\
  \"name\": \"smtp1\",\
  \"host\": \"localhost\",\
  \"from\": \"gareth@domain.tld\",\
  \"to\": \"gareth@domain.tld\",\
  \"subject\": \"message from {source}\",\
  \"body\": \"On {date}, {source} says: {message}, with tags: {tags}\"\
}", JSON_DECODE_ANY, NULL);
  json_t * http_post_alert_valid = json_loads("{\
  \"name\": \"http1\",\
  \"method\": \"POST\",\
  \"url\": \"http://localhost\",\
  \"body\": \"MESSAGE=Message+from+{source}+on+{date},+'{message}'+tags+are:+'{tags}'&Send=SEND\",\
  \"http_headers\": [\
  ]\
}", JSON_DECODE_ANY, NULL);
  json_t * filter_post_valid = json_loads("{\
  \"name\": \"filter1\",\
  \"description\": \"valid test\",\
  \"filter_clauses\": [\
    {\"element\": \"priority\", \"operator\": \">=\", \"value\": 2}\
  ],\
  \"filter_alerts\": {\"smtp\": [\"smtp1\"], \"http\": [\"http1\"]}\
}", JSON_DECODE_ANY, NULL);
  json_t * message_post_high = json_loads("{\
  \"priority\": \"HIGH\",\
  \"source\": \"Francis Underwood\",\
  \"text\": \"I'm just getting started\",\
  \"tags\": [\"minutes\", \"late\"]\
}", JSON_DECODE_ANY, NULL);
  json_t * message_post_low = json_loads("{\
  \"priority\": \"LOW\",\
  \"source\": \"Axel Rose\",\
  \"text\": \"November Rain\",\
  \"tags\": [\"on\", \"you_own\"]\
}", JSON_DECODE_ANY, NULL);

  run_simple_test("POST", SERVER_URL_PREFIX "/alert/smtp/", smtp_post_alert_valid, 200, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/alert/http/", http_post_alert_valid, 200, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/filter/", filter_post_valid, 200, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/message/", message_post_high, 200, NULL);
  run_simple_test("POST", SERVER_URL_PREFIX "/message/", message_post_low, 200, NULL);
  run_simple_test("DELETE", SERVER_URL_PREFIX "/filter/filter1", NULL, 200, NULL);
  run_simple_test("DELETE", SERVER_URL_PREFIX "/alert/smtp/smtp1", NULL, 200, NULL);
  run_simple_test("DELETE", SERVER_URL_PREFIX "/alert/http/http1", NULL, 200, NULL);
  
  json_decref(smtp_post_alert_valid);
  json_decref(http_post_alert_valid);
  json_decref(filter_post_valid);
  json_decref(message_post_high);
  json_decref(message_post_low);
}

int main(int argc, char ** argv) {
  struct _u_request auth_req;
  struct _u_response auth_resp;
  int res;

  ulfius_init_request(&auth_req);
  ulfius_init_response(&auth_resp);
  auth_req.http_verb = strdup("POST");
  auth_req.http_url = msprintf("%s/token/", argc>4?argv[4]:AUTH_SERVER_URI);
  u_map_put(auth_req.map_post_body, "grant_type", "password");
  u_map_put(auth_req.map_post_body, "username", argc>1?argv[1]:USER_LOGIN);
  u_map_put(auth_req.map_post_body, "password", argc>2?argv[2]:USER_PASSWORD);
  u_map_put(auth_req.map_post_body, "scope", argc>3?argv[3]:USER_SCOPE_LIST);
  res = ulfius_send_http_request(&auth_req, &auth_resp);
  if (res == U_OK && auth_resp.status == 200) {
    json_t * json_body = ulfius_get_json_body_response(&auth_resp, NULL);
    token = msprintf("Bearer %s", (json_string_value(json_object_get(json_body, "access_token"))));
    printf("User %s authenticated\n", USER_LOGIN);
    json_decref(json_body);
  } else {
    printf("Error authentication user %s\n", USER_LOGIN);
  }
  printf("Press <enter> to run smtp_alert tests\n");
  getchar();
  run_smtp_alert_tests();
  printf("Press <enter> to run http_alert tests\n");
  getchar();
  run_http_alert_tests();
  printf("Press <enter> to run filter tests\n");
  getchar();
  run_filter_tests();
  printf("Press <enter> to run add_message tests\n");
  getchar();
  run_add_message_tests();
  printf("Press <enter> to run get_messages tests\n");
  getchar();
  run_get_message_tests();
  printf("Press <enter> to run message_trigger_alert tests\n");
  getchar();
  run_message_trigger_alert_tests();
  return 0;
}
