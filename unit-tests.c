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

//#define SERVER_URL_PREFIX "http://localhost:4784/gareth"
#define SERVER_URL_PREFIX "http://localhost:2473/gareth"

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
  char * dump_json = NULL;
  if (response != NULL) {
    printf("Status: %ld\n\n", response->status);
    if (response->json_body != NULL) {
      dump_json = json_dumps(response->json_body, JSON_INDENT(2));
      printf("Json body:\n%s\n\n", dump_json);
      free(dump_json);
    } else if (response->string_body != NULL) {
      printf("String body: %s\n\n", response->string_body);
    }
  }
}


int test_request_status(struct _u_request * req, long int expected_status, json_t * expected_contains) {
  int res, to_return = 0;
  struct _u_response response;
  
  ulfius_init_response(&response);
  res = ulfius_send_http_request(req, &response);
  if (res == U_OK) {
    if (response.status != expected_status) {
      printf("##########################\nError status (%s %s %ld)\n", req->http_verb, req->http_url, expected_status);
      print_response(&response);
      printf("##########################\n\n");
    } else if (expected_contains != NULL && (response.json_body == NULL || json_search(response.json_body, expected_contains) == NULL)) {
      char * dump_expected = json_dumps(expected_contains, JSON_ENCODE_ANY), * dump_response = json_dumps(response.json_body, JSON_ENCODE_ANY);
      printf("##########################\nError json (%s %s)\n", req->http_verb, req->http_url);
      printf("Expected result in response:\n%s\nWhile response is:\n%s\n", dump_expected, dump_response);
      printf("##########################\n\n");
      free(dump_expected);
      free(dump_response);
    } else {
      printf("Success (%s %s %ld)\n\n", req->http_verb, req->http_url, expected_status);
      to_return = 1;
    }
  } else {
    printf("Error in http request: %d\n", res);
  }
  ulfius_clean_response(&response);
  return to_return;
}

void run_smtp_alert_tests() {
  json_t * smtp_post_alert_valid = json_loads("{\
  \"name\": \"smtp1\",\
  \"host\": \"localhost\",\
  \"port\": null,\
  \"tls\": null,\
  \"check_ca\": null,\
  \"user\": null,\
  \"password\": null,\
  \"from\": \"moi\",\
  \"to\": \"lui\",\
  \"cc\": null,\
  \"bcc\": null,\
  \"subject\": \"message de moi\",\
  \"body\": \"a toi et eux\"\
}", JSON_DECODE_ANY, NULL);
  json_t * smtp_post_alert_invalid = json_loads("{\
  \"name\": \"smtp2\",\
  \"host\": \"localhost\",\
  \"port\": \"e\",\
  \"tls\": null,\
  \"check_ca\": null,\
  \"user\": null,\
  \"password\": null,\
  \"from\": \"moi\",\
  \"to\": \"lui\",\
  \"cc\": null,\
  \"bcc\": null,\
  \"subject\": \"message de moi\",\
  \"body\": \"a toi et eux\"\
}", JSON_DECODE_ANY, NULL);
  json_t * smtp_put_alert_valid = json_loads("{\
  \"name\": \"smtp1\",\
  \"host\": \"localhost2\",\
  \"port\": null,\
  \"tls\": true,\
  \"check_ca\": false,\
  \"user\": \"john\",\
  \"password\": \"therupper\",\
  \"from\": \"moi\",\
  \"to\": \"lui\",\
  \"cc\": null,\
  \"bcc\": null,\
  \"subject\": \"message de lui\",\
  \"body\": \"a moi et pas a eux\"\
}", JSON_DECODE_ANY, NULL);
  json_t * smtp_put_alert_invalid = json_loads("{\
  \"name\": \"smtp1\",\
  \"host\": \"localhost\",\
  \"port\": \"e\",\
  \"tls\": null,\
  \"check_ca\": null,\
  \"user\": null,\
  \"password\": null,\
  \"from\": \"moi\",\
  \"to\": \"lui\",\
  \"cc\": null,\
  \"bcc\": null,\
  \"subject\": \"message de moi\",\
  \"body\": \"a toi et eux\"\
}", JSON_DECODE_ANY, NULL);
json_t * smtp_alert_check = json_loads("{\
  \"check_ca\": false,\
  \"name\": \"smtp1\",\
  \"to\": \"lui\",\
  \"user\": null,\
  \"from\": \"moi\",\
  \"tls\": false,\
  \"subject\": \"message de moi\",\
  \"host\": \"localhost\",\
  \"port\": 0,\
  \"cc\": null,\
  \"body\": \"a toi et eux\",\
  \"password\": null,\
  \"bcc\": null\
}", JSON_DECODE_ANY, NULL);
  
  struct _u_request req_list[] = {
    {"GET", SERVER_URL_PREFIX "/alert/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 200
    {"POST", SERVER_URL_PREFIX "/alert/smtp/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, smtp_post_alert_valid, NULL, 0, NULL, 0}, // 200
    {"POST", SERVER_URL_PREFIX "/alert/smtp/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, smtp_post_alert_invalid, NULL, 0, NULL, 0}, // 400
    {"GET", SERVER_URL_PREFIX "/alert/smtp/smtp1", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 200
    {"GET", SERVER_URL_PREFIX "/alert/smtp/smtp2", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 404
    {"PUT", SERVER_URL_PREFIX "/alert/smtp/smtp1", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, smtp_put_alert_valid, NULL, 0, NULL, 0}, // 200
    {"PUT", SERVER_URL_PREFIX "/alert/smtp/smtp1", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, smtp_put_alert_invalid, NULL, 0, NULL, 0}, // 400
    {"DELETE", SERVER_URL_PREFIX "/alert/smtp/smtp2", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 404
    {"DELETE", SERVER_URL_PREFIX "/alert/smtp/smtp1", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0} // 200
  };

  test_request_status(&req_list[0], 200, NULL);
  test_request_status(&req_list[1], 200, NULL);
  test_request_status(&req_list[2], 400, NULL);
  test_request_status(&req_list[3], 200, smtp_alert_check);
  test_request_status(&req_list[4], 404, NULL);
  test_request_status(&req_list[5], 200, NULL);
  test_request_status(&req_list[6], 400, NULL);
  test_request_status(&req_list[7], 404, NULL);
  test_request_status(&req_list[8], 200, NULL);
  
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
  
  struct _u_request req_list[] = {
    {"GET", SERVER_URL_PREFIX "/alert/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 200
    {"POST", SERVER_URL_PREFIX "/alert/http/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, http_post_alert_valid, NULL, 0, NULL, 0}, // 200
    {"POST", SERVER_URL_PREFIX "/alert/http/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, http_post_alert_invalid, NULL, 0, NULL, 0}, // 400
    {"GET", SERVER_URL_PREFIX "/alert/http/http1", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 200
    {"GET", SERVER_URL_PREFIX "/alert/http/http2", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 404
    {"PUT", SERVER_URL_PREFIX "/alert/http/http1", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, http_put_alert_valid, NULL, 0, NULL, 0}, // 200
    {"PUT", SERVER_URL_PREFIX "/alert/http/http1", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, http_put_alert_invalid, NULL, 0, NULL, 0}, // 400
    {"DELETE", SERVER_URL_PREFIX "/alert/http/http2", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 404
    {"DELETE", SERVER_URL_PREFIX "/alert/http/http1", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0} // 200
  };

  test_request_status(&req_list[0], 200, NULL);
  test_request_status(&req_list[1], 200, NULL);
  test_request_status(&req_list[2], 400, NULL);
  test_request_status(&req_list[3], 200, http_post_alert_valid);
  test_request_status(&req_list[4], 404, NULL);
  test_request_status(&req_list[5], 200, NULL);
  test_request_status(&req_list[6], 400, NULL);
  test_request_status(&req_list[7], 404, NULL);
  test_request_status(&req_list[8], 200, NULL);
  
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
  \"port\": null,\
  \"tls\": null,\
  \"check_ca\": null,\
  \"user\": null,\
  \"password\": null,\
  \"from\": \"moi\",\
  \"to\": \"lui\",\
  \"cc\": null,\
  \"bcc\": null,\
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
  
  struct _u_request req_list[] = {
    {"GET", SERVER_URL_PREFIX "/filter/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 200
    {"POST", SERVER_URL_PREFIX "/alert/http/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, http_post_alert_valid, NULL, 0, NULL, 0}, // 200
    {"POST", SERVER_URL_PREFIX "/alert/smtp/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, smtp_post_alert_valid, NULL, 0, NULL, 0}, // 200
    {"POST", SERVER_URL_PREFIX "/filter/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, filter_post_valid, NULL, 0, NULL, 0}, // 200
    {"POST", SERVER_URL_PREFIX "/filter/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, filter_post_invalid, NULL, 0, NULL, 0}, // 400
    {"GET", SERVER_URL_PREFIX "/filter/filter1", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 200
    {"GET", SERVER_URL_PREFIX "/filter/filter2", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 404
    {"PUT", SERVER_URL_PREFIX "/filter/filter1", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, filter_put_valid, NULL, 0, NULL, 0}, // 200
    {"PUT", SERVER_URL_PREFIX "/filter/filter1", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, filter_put_invalid, NULL, 0, NULL, 0}, // 400
    {"DELETE", SERVER_URL_PREFIX "/filter/filter2", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 404
    {"DELETE", SERVER_URL_PREFIX "/filter/filter1", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 200
    {"DELETE", SERVER_URL_PREFIX "/alert/http/httptest1", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 200
    {"DELETE", SERVER_URL_PREFIX "/alert/smtp/smtptest1", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0} // 200
  };

  test_request_status(&req_list[0], 200, NULL);
  test_request_status(&req_list[1], 200, NULL);
  test_request_status(&req_list[2], 200, NULL);
  test_request_status(&req_list[3], 200, NULL);
  test_request_status(&req_list[4], 400, NULL);
  test_request_status(&req_list[5], 200, filter_post_valid);
  test_request_status(&req_list[6], 404, NULL);
  test_request_status(&req_list[7], 200, NULL);
  test_request_status(&req_list[8], 400, NULL);
  test_request_status(&req_list[9], 404, NULL);
  test_request_status(&req_list[10], 200, NULL);
  test_request_status(&req_list[11], 200, NULL);
  test_request_status(&req_list[12], 200, NULL);
  
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
  
  struct _u_request req_list[] = {
    {"POST", SERVER_URL_PREFIX "/message/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, message_post_valid1, NULL, 0, NULL, 0}, // 200
    {"POST", SERVER_URL_PREFIX "/message/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, message_post_valid2, NULL, 0, NULL, 0}, // 200
    {"POST", SERVER_URL_PREFIX "/message/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, message_post_valid3, NULL, 0, NULL, 0}, // 200
    {"POST", SERVER_URL_PREFIX "/message/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, message_post_valid4, NULL, 0, NULL, 0}, // 200
    {"POST", SERVER_URL_PREFIX "/message/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, message_post_valid5, NULL, 0, NULL, 0}, // 200
    {"POST", SERVER_URL_PREFIX "/message/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, message_post_invalid1, NULL, 0, NULL, 0}, // 400
    {"POST", SERVER_URL_PREFIX "/message/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, message_post_invalid2, NULL, 0, NULL, 0}, // 400
    {"POST", SERVER_URL_PREFIX "/message/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, message_post_invalid3, NULL, 0, NULL, 0}, // 400
    {"POST", SERVER_URL_PREFIX "/message/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, message_post_invalid4, NULL, 0, NULL, 0}, // 400
    {"POST", SERVER_URL_PREFIX "/message/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, message_post_invalid5, NULL, 0, NULL, 0}, // 400
    {"GET", SERVER_URL_PREFIX "/message/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 200
  };

  test_request_status(&req_list[0], 200, NULL);
  test_request_status(&req_list[1], 200, NULL);
  test_request_status(&req_list[2], 200, NULL);
  test_request_status(&req_list[3], 200, NULL);
  test_request_status(&req_list[4], 200, NULL);
  test_request_status(&req_list[5], 400, NULL);
  test_request_status(&req_list[6], 400, NULL);
  test_request_status(&req_list[7], 400, NULL);
  test_request_status(&req_list[8], 400, NULL);
  test_request_status(&req_list[9], 400, NULL);
  test_request_status(&req_list[10], 200, NULL);
  
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

  struct _u_request req_list[] = {
    {"POST", SERVER_URL_PREFIX "/filter/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, filter_post_valid1, NULL, 0, NULL, 0}, // 200
    {"POST", SERVER_URL_PREFIX "/filter/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, filter_post_valid2, NULL, 0, NULL, 0}, // 200
    {"POST", SERVER_URL_PREFIX "/filter/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, filter_post_valid3, NULL, 0, NULL, 0}, // 200
    {"POST", SERVER_URL_PREFIX "/filter/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, filter_post_valid4, NULL, 0, NULL, 0}, // 200
    {"GET", SERVER_URL_PREFIX "/message/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 200
    {"GET", SERVER_URL_PREFIX "/message/filter1", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 200
    {"GET", SERVER_URL_PREFIX "/message/filter2", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 200
    {"GET", SERVER_URL_PREFIX "/message/filter3", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 200
    {"GET", SERVER_URL_PREFIX "/message/filter4", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 200
    {"DELETE", SERVER_URL_PREFIX "/filter/filter1", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 200
    {"DELETE", SERVER_URL_PREFIX "/filter/filter2", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 200
    {"DELETE", SERVER_URL_PREFIX "/filter/filter3", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 200
    {"DELETE", SERVER_URL_PREFIX "/filter/filter4", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 200
  };

  test_request_status(&req_list[0], 200, NULL);
  test_request_status(&req_list[1], 200, NULL);
  test_request_status(&req_list[2], 200, NULL);
  test_request_status(&req_list[3], 200, NULL);
  test_request_status(&req_list[4], 200, NULL);
  test_request_status(&req_list[5], 200, NULL);
  test_request_status(&req_list[6], 200, NULL);
  test_request_status(&req_list[7], 200, NULL);
  test_request_status(&req_list[8], 200, NULL);
  test_request_status(&req_list[9], 200, NULL);
  test_request_status(&req_list[10], 200, NULL);
  test_request_status(&req_list[11], 200, NULL);
  test_request_status(&req_list[12], 200, NULL);
  
  json_decref(filter_post_valid1);
  json_decref(filter_post_valid2);
  json_decref(filter_post_valid3);
  json_decref(filter_post_valid4);
}

void run_message_trigger_alert_tests() {
  json_t * smtp_post_alert_valid = json_loads("{\
  \"name\": \"smtp1\",\
  \"host\": \"lohot\",\
  \"port\": null,\
  \"tls\": null,\
  \"check_ca\": null,\
  \"user\": null,\
  \"password\": null,\
  \"from\": \"gareth@babelouest.org\",\
  \"to\": \"nicolas@babelouest.org\",\
  \"cc\": null,\
  \"bcc\": null,\
  \"subject\": \"message de {source}\",\
  \"body\": \"Le {date}, {source} te dit: {message}, avec les tags: {tags}\"\
}", JSON_DECODE_ANY, NULL);
  json_t * http_post_alert_valid = json_loads("{\
  \"name\": \"http1\",\
  \"method\": \"POST\",\
  \"url\": \"http://msg.koodomobile.com/msg/HTTPPostMgr\",\
  \"body\": \"CODE=418&NUM=2552972&CALLBACK=4182552972&MESSAGE=Message+de+{source}+qui+te+dit+le+{date},+'{message}'+les+tags+sont:+'{tags}'&Send=SEND\",\
  \"http_headers\": [\
    {\"key\": \"Host\", \"value\": \"msg.koodomobile.com\"},\
    {\"key\": \"Content-Type\", \"value\": \"application/x-www-form-urlencoded\"}\
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

  struct _u_request req_list[] = {
    {"POST", SERVER_URL_PREFIX "/alert/smtp/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, smtp_post_alert_valid, NULL, 0, NULL, 0}, // 200
    {"POST", SERVER_URL_PREFIX "/alert/http/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, http_post_alert_valid, NULL, 0, NULL, 0}, // 200
    {"POST", SERVER_URL_PREFIX "/filter/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, filter_post_valid, NULL, 0, NULL, 0}, // 200
    {"POST", SERVER_URL_PREFIX "/message/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, message_post_high, NULL, 0, NULL, 0}, // 200
    {"POST", SERVER_URL_PREFIX "/message/", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, message_post_low, NULL, 0, NULL, 0}, // 200
    {"DELETE", SERVER_URL_PREFIX "/filter/filter1", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 200
    {"DELETE", SERVER_URL_PREFIX "/alert/smtp/smtp1", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 200
    {"DELETE", SERVER_URL_PREFIX "/alert/http/http1", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0}, // 200
  };

  test_request_status(&req_list[0], 200, NULL);
  test_request_status(&req_list[1], 200, NULL);
  test_request_status(&req_list[2], 200, NULL);
  test_request_status(&req_list[3], 200, NULL);
  test_request_status(&req_list[4], 200, NULL);
  test_request_status(&req_list[5], 200, NULL);
  test_request_status(&req_list[6], 200, NULL);
  test_request_status(&req_list[7], 200, NULL);
  
  json_decref(smtp_post_alert_valid);
  json_decref(http_post_alert_valid);
  json_decref(filter_post_valid);
  json_decref(message_post_high);
  json_decref(message_post_low);
}

int main(void) {
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
