#!/usr/bin/perl

# (C) Dmitry Volyntsev
# (C) Nginx, Inc.

# Tests for http njs module, r.requestText method.

###############################################################################

use warnings;
use strict;

use Test::More;

use Socket qw/ CRLF /;

BEGIN { use FindBin; chdir($FindBin::Bin); }

use lib 'lib';
use Test::Nginx;

###############################################################################

select STDERR; $| = 1;
select STDOUT; $| = 1;

my $t = Test::Nginx->new()->has(qw/http/)
	->write_file_expand('nginx.conf', <<'EOF');

%%TEST_GLOBALS%%

daemon off;

events {
}

http {
    %%TEST_GLOBALS_HTTP%%

    js_import test.js;

    server {
        listen       127.0.0.1:8080;
        server_name  localhost;

        location /body {
            js_content test.body;
        }

        location /body_4k {
            client_body_buffer_size 4k;
            js_content test.body;
        }

        location /in_file {
            js_content test.body;
        }

        location /in_file_on {
            client_body_in_file_only on;
            js_content test.body;
        }

        location /in_file_clean {
            client_body_in_file_only clean;
            js_content test.body;
        }

        location /read_body_from_temp_file {
            client_body_in_file_only clean;
            js_content test.read_body_from_temp_file;
        }

        location /request_body_cache {
            js_content test.request_body_cache;
        }
    }
}

EOF

$t->write_file('test.js', <<EOF);
    import fs from 'fs';

    function body(r) {
        try {
            var body = r.requestText;
            r.return(200, body);

        } catch (e) {
            r.return(500, e.message);
        }
    }

    function read_body_from_temp_file(r) {
        let fn = r.variables.request_body_file;
        r.return(200, fs.readFileSync(fn));
    }

    function request_body_cache(r) {
        function t(v) {return Buffer.isBuffer(v) ? 'buffer' : (typeof v);}
        r.return(200,
      `requestText:\${t(r.requestText)} requestBuffer:\${t(r.requestBuffer)}`);
    }

    export default {body, read_body_from_temp_file, request_body_cache};

EOF

$t->try_run('no njs request body')->plan(9);

###############################################################################

like(http_post('/body'), qr/REQ-BODY/, 'request body');
like(http_post('/in_file'), qr/REQ-BODY/, 'request body in a file');
like(http_post('/in_file_on'), qr/REQ-BODY/, 'request body in a file on');
like(http_post('/in_file_clean'), qr/REQ-BODY/, 'request body in a file clean');
like(http_post_big('/body'), qr/200.*^(1234567890){1024}$/ms,
		'request body big');
like(http_post_big('/body_4k'), qr/200.*^(1234567890){1024}$/ms,
		'request body big with 4k buffer');
like(http_post_big('/read_body_from_temp_file'),
	qr/200.*^(1234567890){1024}$/ms, 'request body big from temp file');
like(http_post('/request_body_cache'),
	qr/requestText:string requestBuffer:buffer$/s, 'request body cache');

$t->stop();

ok(index($t->read_file('error.log'),
	'http js reading request body from a temporary file') > 0,
	'http request body is in a file warning');

###############################################################################

sub http_post {
	my ($url, %extra) = @_;

	my $p = "POST $url HTTP/1.0" . CRLF .
		"Host: localhost" . CRLF .
		"Content-Length: 8" . CRLF .
		CRLF .
		"REQ-BODY";

	return http($p, %extra);
}

sub http_post_big {
	my ($url, %extra) = @_;

	my $p = "POST $url HTTP/1.0" . CRLF .
		"Host: localhost" . CRLF .
		"Content-Length: 10240" . CRLF .
		CRLF .
		("1234567890" x 1024);

	return http($p, %extra);
}

###############################################################################
