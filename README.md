# Gareth

Log application and messenging using a Json/REST interface.

Part of the Angharad System, this program is intended to log messages from Angharad elements, and can send an http request or an smtp message if a trigger has been set for the message.

A message has the following attributes:
- A priority (none, low, medium, high or critical)
- A source (Who sent the message ?)
- A text message (What is is about ?)
- A set of tags

You can see the logs, and filter your search with attributes, you can also display the messages corresponding to a named filter that has its criteria. Finally, you can attach an alert to a named filter, so if a message enters and correspond to a filter, an alert will be sent.

An alert is technically a smtp message or a http request. A http request can be used to send sms or call other webservices for examples.

# Prerequisites

Gareth depend on the libraries [Ulfius](https://github.com/babelouest/ulfius/), [Yder](https://github.com/babelouest/yder/), [Hoel](https://github.com/babelouest/hoel/), [Orcania](https://github.com/babelouest/orcania/) for the Angharad libraries, also [Jansson](http://www.digip.org/jansson/) and [Libconfig](http://www.hyperrealm.com/libconfig/) for the non Angharad libraries.

You must install the dependencies (and their dependencies too) before installing Gareth.

You must use a MariaDB/Mysql database or a SQLite database.

# Standalone Installation

Compile gareth as a standalone application:

```shell
$ make release-standalone
```

# Configuration

You must specify a configuration file when you start gareth-standalone. You can copy the file `gareth.conf.sample` to `gareth.conf` and modify it to fit your needs.

# Start and stop Gareth

This is the online help for gareth:

```shell
$ gareth-standalone -h
Gareth Messaging REST Webservice

Messaging system using a JSON/REST interface

-c --config-file=PATH
	Path to configuration file
-p --port=PORT
	Port to listen to
-u --url-prefix=PREFIX
	URL prefix
-m --log-mode=MODE
	Log Mode
	console, syslog or file
	If you want multiple modes, separate them with a comma ","
	default: console
-l --log-level=LEVEL
	Log level
	NONE, ERROR, WARNING, INFO, DEBUG
	default: ERROR
-f --log-file=PATH
	Path for log file if log mode file is specified
-h --help
	Print this message
```

All those parameters can be specified in the configuration file.

To start gareth-standalone via the command-line, enter the command:

```shell
$ ./gareth-standalone --config-file=gareth.conf
```

Right after start, your log should see in the logs:

```
2016-01-14 18:54:20 - Gareth INFO: Starting Gareth alert and messenger service 
2016-01-14 18:54:20 - Gareth INFO: Start gareth on port 4784, prefix: gareth 
```

The service is now started and can be called using the HTTP REST API interface.

# REST API Documentation

In the documentation, we assume you use the prefix `/gareth`.

For all endpoints, return status can be 200 for success, 400 for invalid parameters, 404 for resource or page not found, or 500 for internal server error. On an error 400, the json body result explains the error.

## Message management APIs

### POST /gareth/message

Post a new message.

#### Parameters

##### URL

None

##### Body

```javascrit
{
  "priority": string, values available: NONE, LOW, MEDIUM, HIGH, CRITICAL
  "source": string, maximum 64 characters
  "text": string, maximum 256 characters
  "tags": array of strings, maximum 16 tags, and maximum 32 characters per tag
}
```

##### Result body

None

### GET /gareth/message/@filter_name

Get a list of messages, filter result using a filter name, or message attributes criteria

#### Parameters

##### URL

- @filter_name: string, name of an existing filter. If not set, all messages are displayed
- date: integer, filter messages younger than date in seconds
- source: string filter messages from source parameter
- priority: string, filter messages that have the corresponding priority
- text: string, filter messages that contain the filter value
- tags: string, filter messages that contain the specified tag
- limit: integer, specify the maximal number of message to display, limit can't be hogher than 100. If no specified, limit is set to 20 messages
- offset: integer, specify the offset message to start the search

##### Body

None

##### Result Body

An array of messages. A message has the following format:

```javascrit
{
  "date": string
  "priority": string, values available: NONE, LOW, MEDIUM, HIGH, CRITICAL
  "source": string
  "text": string
  "tags": array of strings
}
```

### Message trigger

If a message correspond to a filter that has alerts (see below), this will trigger a message to be send.

For example, let's say you have this following smtp alert:

```javascrit
{
  "name": "smtp1",
  "host": "localhost",
  "port": null,
  "tls": null,
  "check_ca": null,
  "user": null,
  "password": null,
  "from": "gareth@mail.tld",
  "to": "user@mail.tld",
  "cc": "archive@mail.tld",
  "bcc": null,
  "subject": "Alert message from {source}",
  "body": "On {date}, {source} has sent the following alert: {message}\n\nPriority: {priority}\nAdditional tags: {tags}"
}
```

Then, Gareth has received the following message:

```javascrit
{
  "priority": "HIGH",
  "source": "Door Sensor",
  "text": "The front door has been opened",
  "tags": ["front", "door"]
}
```

The triggered message will have the following form:

```
From: gareth@mail.tld
To: user@mail.tld
Cc: archive@mail.tld
Subject: Alert message from Door Sensor

On 2016/01/14 18:21:12, Door Sensor has sent the following alert: The front door has been opened

Priority: High
Additional tags: front, door
```

## Alerts management APIs

An alert is used joined with one or several filters, when a message is posted and matches a filter (e.g. a message with priority `CRITICAL` from the source `door sensor`), a message alert is triggered.

An alert can be either a smtp message (email) or http request (to call an sms api from your provider for example).

An smtp alert has the following format:

```javascrit
{
  "name": name of the smtp alert, must be unique, string, max 64 characters, mandatory
  "host": smtp hostname, string, max 128 characters, mandatory
  "port": smtp port number, integer, between 0 and 65535, optional, if set to 0 or not set, default port number will be used (25 or 587, depending on tls set to true or not)
  "tls": use TLS to connect, boolean, optional, default false
  "check_ca": check the certificate authority of the smtp host if tls is activated, boolean, optional, default false
  "user": username to connect to the smtp host, string, max 128 characters, optional
  "password": password to connect to the smtp host, string, max 128 characters, optional
  "from": email address of the sender, string, max 128 characters, mandatory
  "to": email address of the recipient, string, max 128 characters, mandatory
  "cc": email address of the recipient in cc, string, max 128 characters, optional
  "bcc": email address of the recipient in bcc, string, max 128 characters, optional
  "subject": subject of the email, string, max 128 characters, mandatory
  "body": body of the email, string, max 512 characters, optional
}
```

`subject` and `body` can contain some specific patterns:
- {priority}: will be replaced by the priority of the message
- {source}: will be replaced by the source of the message
- {message}: will be replaced by the text of the message
- {tags}: will be replaced by the list of tags of the message, separated by a comma
- {date}: will be replaced by the date of the message, using the format format `YYYY/MM/DD hh:mm:ss`

An http alert has the following format:

```javascrit
{
  "name": name of the http alert, must be unique, string, max 64 characters, mandatory
  "method": http method (GET, POST, PUT, etc.), string, max 16 characters, optional (default GET)
  "url": url to call, string, max 128 characters, mandatory
  "body": body of the http call, max 512 characters, optional
  "http_headers": [ Array of http headers to add to the alert, optional
    {
      "key": key of the http header, max 64 characters, mandatory
      "value": value of the http header, max 128 characters, mandatory
    }
  ]
}
```

The attributes `url`, `body`, `key` or `value` can contain patterns as described above:
- {priority}: will be replaced by the priority of the message
- {source}: will be replaced by the source of the message
- {message}: will be replaced by the text of the message
- {tags}: will be replaced by the list of tags of the message, separated by a coma
- {date}: will be replaced by the date of the message, using the format format `YYYY/MM/DD hh:mm:ss`

If specified in the url and the body, these patterns will be url encoded according to the [RFC 3986](https://tools.ietf.org/html/rfc3986).

### GET /gareth/alert

Get all alerts for all types (http and smtp)

#### Parameters

##### URL

None

##### Body

None

##### Result Body

An object with 2 attributes: an array of smtp alerts and an array of http alerts:

```javascrit
{
  "smtp": [
    {
      "name": name of the smtp alert, string, max 64 characters
      "host": smtp hostname, string, max 128 characters
      "port": smtp port number, integer, between 0 and 65535, if set to 0, default port number will be used (25 or 587, depending on tls set to true or not)
      "tls": use TLS to connect, boolean
      "check_ca": check the certificate authority of the smtp host if tls is activated, boolean
      "user": username to connect to the smtp host, string, max 128 characters or null
      "password": password to connect to the smtp host, string, max 128 characters or null
      "from": email address of the sender, string, max 128 characters
      "to": email address of the recipient, string, max 128 characters
      "cc": email address of the recipient in cc, string, max 128 characters
      "bcc": email address of the recipient in bcc, string, max 128 characters
      "subject": subject of the email, string, max 128 characters
      "body": body of the email, string, max 512 characters
    }
  ],
  "http": [
    {
      "name": name of the http alert, string, max 64 characters
      "method": http method (GET, POST, PUT, etc.), string, max 16 characters
      "url": url to call, string, max 128 characters
      "body": body of the http call, max 512 characters
      "http_headers": [
        {
          "key": key of the http header, max 64 characters
          "value": value of the http header, max 128 characters
        }
      ]
    }
  ]
}
```

### GET /gareth/alert/@type/@alert_name

Get a apecific alert of the specified type and name

#### Parameters

##### URL

- @type: smtp or http
- @alert_name: name of the alert

##### Body

None

##### Result Body

smtp alert:

```javascrit
{
  "name": name of the smtp alert, string, max 64 characters
  "host": smtp hostname, string, max 128 characters
  "port": smtp port number, integer, between 0 and 65535, if set to 0, default port number will be used (25 or 587, depending on tls set to true or not)
  "tls": use TLS to connect, boolean
  "check_ca": check the certificate authority of the smtp host if tls is activated, boolean
  "user": username to connect to the smtp host, string, max 128 characters or null
  "password": password to connect to the smtp host, string, max 128 characters or null
  "from": email address of the sender, string, max 128 characters
  "to": email address of the recipient, string, max 128 characters
  "cc": email address of the recipient in cc, string, max 128 characters
  "bcc": email address of the recipient in bcc, string, max 128 characters
  "subject": subject of the email, string, max 128 characters
  "body": body of the email, string, max 512 characters
}
```

http alert:

```javascrit
{
  "name": name of the http alert, string, max 64 characters
  "method": http method (GET, POST, PUT, etc.), string, max 16 characters
  "url": url to call, string, max 128 characters
  "body": body of the http call, max 512 characters
  "http_headers": [
    {
      "key": key of the http header, max 64 characters
      "value": value of the http header, max 128 characters
    }
  ]
}
```

### POST /gareth/alert/@type/

Add a new alert of the specified type

#### Parameters

##### URL

- @type: smtp or http

##### Body

smtp alert:
```javascrit
{
  "name": name of the smtp alert, must be unique, string, max 64 characters, mandatory
  "host": smtp hostname, string, max 128 characters, mandatory
  "port": smtp port number, integer, between 0 and 65535, optional, if set to 0 or not set, default port number will be used (25 or 587, depending on tls set to true or not)
  "tls": use TLS to connect, boolean, optional, default false
  "check_ca": check the certificate authority of the smtp host if tls is activated, boolean, optional, default false
  "user": username to connect to the smtp host, string, max 128 characters, optional
  "password": password to connect to the smtp host, string, max 128 characters, optional
  "from": email address of the sender, string, max 128 characters, mandatory
  "to": email address of the recipient, string, max 128 characters, mandatory
  "cc": email address of the recipient in cc, string, max 128 characters, optional
  "bcc": email address of the recipient in bcc, string, max 128 characters, optional
  "subject": subject of the email, string, max 128 characters, mandatory
  "body": body of the email, string, max 512 characters, optional
}
```

http alert:
```javascrit
{
  "name": name of the http alert, must be unique, string, max 64 characters, mandatory
  "method": http method (GET, POST, PUT, etc.), string, max 16 characters, optional (default GET)
  "url": url to call, string, max 128 characters, mandatory
  "body": body of the http call, max 512 characters, optional
  "http_headers": [ Array of http headers to add to the alert, optional
    {
      "key": key of the http header, max 64 characters, mandatory
      "value": value of the http header, max 128 characters, mandatory
    }
  ]
}
```

##### Result Body

None

### PUT /gareth/alert/@type/@alert_name

Modify an existing alert of the specified type

#### Parameters

##### URL

- @type: smtp or http
- @alert_name: name of the alert

##### Body

smtp alert:
```javascrit
{
  "host": smtp hostname, string, max 128 characters, mandatory
  "port": smtp port number, integer, between 0 and 65535, optional, if set to 0 or not set, default port number will be used (25 or 587, depending on tls set to true or not)
  "tls": use TLS to connect, boolean, optional, default false
  "check_ca": check the certificate authority of the smtp host if tls is activated, boolean, optional, default false
  "user": username to connect to the smtp host, string, max 128 characters, optional
  "password": password to connect to the smtp host, string, max 128 characters, optional
  "from": email address of the sender, string, max 128 characters, mandatory
  "to": email address of the recipient, string, max 128 characters, mandatory
  "cc": email address of the recipient in cc, string, max 128 characters, optional
  "bcc": email address of the recipient in bcc, string, max 128 characters, optional
  "subject": subject of the email, string, max 128 characters, mandatory
  "body": body of the email, string, max 512 characters, optional
}
```

http alert:
```javascrit
{
  "method": http method (GET, POST, PUT, etc.), string, max 16 characters, optional (default GET)
  "url": url to call, string, max 128 characters, mandatory
  "body": body of the http call, max 512 characters, optional
  "http_headers": [ Array of http headers to add to the alert, optional
    {
      "key": key of the http header, max 64 characters, mandatory
      "value": value of the http header, max 128 characters, mandatory
    }
  ]
}
```

##### Result Body

None

### DELETE /gareth/alert/@type/@alert_name

Delete an existing alert of the specified type

#### Parameters

##### URL

- @type: smtp or http
- @alert_name: name of the alert

##### Body

None

##### Result Body

None

## Filters management APIs

Filters can be used for both filter a message list or trigger an smtp or http message when a message matches the filter.

A filter has the following format:

```javascrit
  "name": name of the filter, string, max 
  "description": "valid test",
  "filter_clauses": [
    {"element": "priority", "operator": ">=", "value": 2}
  ],
  "filter_alerts": {"smtp": ["smtp1"], "http": ["http1"]}
```
