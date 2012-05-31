# -*- mode: qore; indent-tabs-mode: nil -*-
# SOAP handler class definition
#
# to be registered as a handler to the Qore HTTP server

# v0.2.3: return correct soap fault structure according to request
# v0.2.2: soap 1.2 improvements
# v0.2.1: matched to new WSDL and HttpServer implementation
# v0.2.0: better WSDL support
# v0.1.0: initial WSDL-based SOAP support (still incomplete)

class SoapHandler inherits public AbstractHttpRequestHandler {
    const Version = "0.2.3";

    private {
        hash $.methods;
        $.mi;

        # hash of WebService objects (values) per path (keys)
        hash $.ws;

        # hash: path -> top-level-method-name -> method
        hash $.pathmethods;

        int $.loglevel;
    }

    constructor(AbstractAuthenticator $auth, *list $methods) : AbstractHttpRequestHandler($auth) {
	#foreach my $m in (InternalMethods)
	#    $.addMethodInternal($m);

	foreach my $m in ($methods) {
	    if (!($m.operation instanceof WSOperation))
		throw "SOAP-CONSTRUCTOR-ERROR", sprintf("expecting 'operation' key in method hash to be an instance of class WSOperation (%n)", $m);
	    if (!exists $m.function)
		throw "SOAP-CONSTRUCTOR-ERROR", sprintf("expecting 'function' key in method hash (%n)", $m);

	    delete $m.internal;
	    $.addMethodInternal($m);
	}
    }

    addMethod(WebService $ws, WSOperation $op, $func, $help, $logopt, $cmark, $path) {
	if (!exists $func)
	    throw "SOAP-SERVER-ADDMETHOD-PARAMETER-ERROR", "second argument is not a function name";

	$.addMethodInternal($ws,
			    ( "operation" : $op,
			      "name"      : $op.name,
			      "function"  : $func,
			      "help"      : $help,
			      "logopt"    : $logopt,
			      "cmark"     : $cmark,
			      "path"      : $path));
	#printf("methods=%N\n", $.methods);
    }

    private addMethodInternal(WebService $ws, $method) {
	my string $rn = $method.operation.getTopLevelRequestName();

	# map top-level element to method
	if (exists $method.path) {
	    my $path = $method.path;

	    # add link to WebService object
	    if (!exists $.ws.$path)
		$.ws.$path = $ws;

	    $.pathmethods.$path.$rn = $method;

	    # add method link if it doesn't already exist
	    if (!exists $.methods.$rn)
		$.methods.$rn = $method;
	}
	else
	    $.methods.$rn = $method;

	#printf("DEBUG: addMethodInternal() %N %N\n", $method.name, $method.path);
    }

    private help($c) {
	my $h;
	foreach my string $cmd in (keys $.methods) {
	    $h.$cmd.description = $.methods.$cmd.help;
	    if (exists $.methods.$cmd.params)
		$h.$cmd.params  = $.methods.$cmd.params;
	}
	foreach my string $path in (keys $.pathmethods) {
	    foreach my $cmd in (keys $.pathmethods.$path) {
		$h.$cmd.description = $.methods.$cmd.help;
		if (exists $.pathmethods.$path.$cmd.params)
		    $h.$path.$cmd.params  = $.pathmethods.$path.$cmd.params;
	    }
	}
	return $h;
    }

    private log($cx, $str) {
        my $msg = "SOAP ";
        if (exists $cx.user)
            $msg += sprintf("user %s ", $cx.user);
        $msg += sprintf("from %s: ", $cx."peer-info".address_desc);
        $msg += vsprintf($str, $argv);

        call_function_args($cx.logfunc, $msg);
    }

    private static makeSoapFaultResponse($soap12, $err, $desc) {
	my $o;
	if ($soap12) {
	    $o = ENVELOPE_12_NS;
	    $o."soapenv:Envelope"."soapenv:Body" =
		( "soapenv:Fault" :
		  ( "soapenv:Code" :
		    ( "soapenv:Value" : $err ),
		    "soapenv:Reason" : 
		    ( "soapenv:Text" : $desc ) ) );
	}
	else {
	    $o = ENVELOPE_11_NS;
	    $o."soapenv:Envelope"."soapenv:Body" =
		( "soapenv:Fault" :
		  ( "faultcode"   : $err,
		    "faultstring" : $desc,
		    "desc"        : "" ) );
	}

	return makeXMLString($o);
    }

    private callOperation($cx, $args) {
	my $element = $cx.element;

	my $method;
	if (exists $cx.url.path) {
	    my $path = $cx.url.path;
	    # prepend "/" if it's not already there
	    if ($path !~ /^\//)
		$path =~ s/(.*)/\/$1/;

	    $method = $.pathmethods.$path.$element;
	    #printf("DEBUG: path=%n method exists=%n (%n)\n", $path, exists $method, keys $.pathmethod);
	}

	if (!exists $method)
	    $method = $.methods.$element;

	#printf("DEBUG: found element %N method %N\n", $element, $method);
	if (!exists $method) {
	    if (!exists $element)
		throw "SOAP-SERVER-UNKNOWN-OPERATION", sprintf("cannot find operation element in SOAP call; call context=%N", $cx);	    
	    throw "SOAP-SERVER-UNKNOWN-OPERATION", sprintf("cannot map top-level element %n to a SOAP operation: %s", $element, elements $.methods ? sprintf("currently recognized top-level elements: %n", keys $.methods) : "no SOAP services are currently registered");
	}

	# NOTE: internal methods have no operation definition and can take no parameters
	my $h;
	if ($method.internal) {
	    $h.body = callObjectMethod($self, $method.function);
	    return $h;
	}

	my $soap12 = $method.operation.isSoap12();

	try {
	    $args = $method.operation.deserializeRequest($args);
	}
	catch ($ex) {
	    return ( "errlog" : sprintf("%s:%d: %s: %s", $ex.file, $ex.line, $ex.err, $ex.desc),
		     "body"   : SoapHandler::makeSoapFaultResponse($soap12, $ex.err, $ex.desc),
		     "hdr"    : ( "Content-Type" : $soap12 ? MimeTypeSoapXml : MimeTypeXml ) );
	}

	# add context marker, if any
	$cx.cmark = $method.cmark;
	$cx.function = $method.function;
	$cx.operation = $method.operation;

	if (($method.logopt & HttpServer::LP_LEVELMASK) <= $.loglevel && exists $cx.logfunc) {
	    my $msg;

	    $msg = sprintf("SOAP call from %s: %s", $cx."peer-info".address_desc, $method.name);
	    if ($method.logopt & HttpServer::LP_LOGPARAMS) {
		$msg += sprintf("(");
		my $i = 0;
		foreach my $arg in ($args) {
		    if (inlist($i++, $method.maskargs))
			$msg += "<masked>, ";
		    else if ($arg.typeCode() == NT_HASH && elements $arg) {
			$msg += "(";
			foreach my $k in (keys $arg) {
			    if ($k == $method.maskkey)
				$msg += sprintf("%s=<masked>, ", $k);
			    else
				$msg += sprintf("%s=%n, ", $k, $arg.$k);
			}
			splice $msg, -2, 2;
			$msg += "), ";
		    }
		    else
			$msg += sprintf("%n, ", $arg);
		}
		# remove the last two characters from the string if any were added
		if ($i)
		    splice $msg, -2, 2;
		$msg += ")";
	    }

	    $.log($cx, $msg);
	    $h.logreply = True;
	}
	#printf("about to call function '%s' (method=%s params=%N)\n", $method.function, $method.name, $args);flush();
	my $rv;

	if ($args.typeCode() == NT_LIST)
	    unshift $args, $cx;
	else if (exists $args)
	    $args = ($cx, $args);
	else
	    $args = $cx;

	if ($method.internal)
	    $rv = callObjectMethodArgs($self, $method.function, $args);
	else
	    $rv = call_function_args($method.function, $args);

	# if there is an output message, then serialize the response
	if (exists $method.operation.output)
	    $h += $method.operation.serializeResponse($rv);

	#$.log($cx, "body=%N", $h.body);
	
	return $h;
    }

    # method called by HttpServer
    hash handleRequest(hash $cx, hash $hdr, *data $body) {
	#log(LL_DEBUG_1, "soap handler context=%n hdr=%n body=%n ws=%n", $cx, $hdr, $body, keys $.ws);
	#printf("soap handler context=%y hdr=%y body=%y keys ws=%y\n", $cx, $hdr, $body, keys $.ws);
	$cx.http_header = $hdr;
	$cx.http_body = $body;

	my $args;
	my $reqsoap12; # set to True if soap 1.2 envelope is used in request

	if ($hdr.method == "GET") {
	    my *string $path = $cx.url.path;

	    # prepend "/" if it's not already there
	    if ($path !~ /^\//)
		$path = "/" + $path;

	    # remove ?wsdl if there
	    $path =~ s/\?wsdl$//i;

	    #printf("DEBUG: PATH=%N (%n %n)\n", $path, exists $.ws.$path, keys $.ws);
	    
	    if (!strlen($path))
		return ( "code" : 501,
			 "desc" : "invalid HTTP GET: no path given in URL" ); 

	    if (!exists $.ws.$path)
		return ( "code" : 501,
			 "desc" : sprintf("invalid HTTP GET: no WebService object for path %n", $path) ); 
	    
	    return ( "code" : 200,
		     "body" : $.ws.$path.getWSDL(),
		     "hdr"    : ( "Content-Type" : "text/xml" ) );
	}
	else {
	    if ($hdr.method != "POST")
		return ( "code" : 501,
			 "body" : sprintf("don't know how to handle method %n", $hdr.method) );

	    try {
		my $ct = $hdr."_qore_orig_content_type";
		# parse multipart messages
		if ($ct =~ /multipart\//) {
		    my $x = ($ct =~ x/multipart\/([^;]+)/)[0];
		    $hdr."_qore_multipart" = $x;
		    $x = ($ct =~ x/start=([^;]+)/)[0];
		    if (exists $x)
			$hdr."_qore_multipart_start" = $x;
		    $x = ($ct =~ x/boundary=([^;]+)/)[0];
		    if (exists $x)
			$hdr."_qore_multipart_boundary" = $x;
		    #printf("hdr=%N\n", $hdr);
		}

		$args = WSDLLib::parseSOAPMessage($hdr + ("body" : $body));
		#$args = parseXMLAsData($body);
		WSDL::XSDBase::removeNS(\$args);
		WSDL::XSDBase::removeNS(\$args.Envelope);

		# get SOAP operation name
		my $element;
		if (!exists ($element = ($ct =~ x/action=".*\/(.*)"/)[0])) {

		    my $sbody = $args.Envelope.Body;
		    if (!exists $sbody)
			throw "SOAP-CALL-ERROR", "missing SOAP body in SOAP envelope in SOAP operation call";

		    foreach my $k in (keys $sbody) {
			if ($k == "multiRef" || $k == "^attributes^")
			    continue;
			$element = $k =~ x/.*:(.*)/[0];
			break;
		    }
		}

		# set soap version in request
		my $attr = $args.Envelope."^attributes^";
		foreach my $k in (keys $attr) {
		    if ($k =~ /:soapenv$/) {
			if ($attr.$k == SOAP_12_ENV)
			    $reqsoap12 = True;
			else if ($attr.$k != SOAP_11_ENV)
			    throw "SOAP-CALL-ERROR", sprintf("unsupported SOAP envelope received: %n", $attr.$k);
			break;
		    }
		}
		    
		if (!exists $element)
		    throw "SOAP-CALL-ERROR", "no operation call found in message";
		
		$cx.element = $element;
	    }
	    catch ($ex) {
		log(LL_DETAIL_2, "exception parsing SOAP msg: %n", $body);
		#my string $str = sprintf("exception in %s:%d: %s: %s (1: %N)", $ex.file, $ex.line, $ex.err, $ex.desc, $ex.callstack);
		return ( "code"   : 200,
			 "errlog" : sprintf("%s: %s", $ex.err, $ex.desc),
			 "body"   : SoapHandler::makeSoapFaultResponse($reqsoap12, $ex.err, $ex.desc),
			 "hdr"    : ( "Content-Type" : MimeTypeSoapXml )
		    );
	    }
	}
	try {
	    #printf("DEBUG: context: %N\nargs: %N\n", $cx, $args);
	    return ( "code" : 200 ) + $.callOperation($cx, $args);
	}
	catch ($ex) {
	    my $str = sprintf("exception in %s:%d: %s: %s (2: %N)", $ex.file, $ex.line, $ex.err, $ex.desc, $ex.callstack);
	    return ( "code"   : 200,
		     "errlog" : $str,
		     "body"   : SoapHandler::makeSoapFaultResponse($reqsoap12, $ex.err, $ex.desc),
		     "hdr"    : ( "Content-Type" : MimeTypeSoapXml )
		);
	}
    }
}
