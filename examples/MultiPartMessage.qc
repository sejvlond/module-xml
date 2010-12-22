# -*- mode: qore; indent-tabs-mode: nil -*-
#! @file MultiPartMessage.qc classes to assist with sending HTTP multipart messages

# FIXME: implement case-insensitive header matches
#! MultiPartMessage class implementation
class MultiPartMessage {
    const MPT_FORM_DATA   = "form-data";
    const MPT_MIXED       = "mixed";
    const MPT_MESSAGE     = "message";
    const MPT_DIGEST      = "digest";
    const MPT_ALTERNATIVE = "alternative";
    const MPT_RELATED     = "related";
    const MPT_SIGNED      = "signed";
    const MPT_ENCRYPTED   = "encrypted";
    const MPT_BYTERANGES  = "byteranges";

    #const MPT_INFO = 
    #    ( MPT_MIXED  : ( "default" : ( "Content-Type" : "text/plain" ) ),
    #      MPT_DIGEST : ( "default" : ( "Content-Type" : "message/rfc822" ) ),
    #    );

    const MP_DEFAULT_MSG = "This is a MIME multipart message";

    const MP_QORE_BOUNDARY = "------Qore-MIME-Boundary------";

    public {
        any $.mptype;
        string $.boundary;
        string $.ct;
        list $.l = ();
        hash $.hdr;
    }

    constructor(string $mptype, string $boundary = MultiPartMessage::MP_QORE_BOUNDARY) {
	$.mptype = $mptype;
        $.boundary = $boundary;
	$.ct = sprintf("multipart/related;boundary=%s", $.boundary);
	$.boundary = "--" + $.boundary;
	#printf("DEBUG: %N\n", $self);exit();
    }

    static hash getPart(any $data, hash $hdr) {
	if (exists $hdr."Content-Length")
	    throw "HTTP-MULTIPART-ERROR", "illegal part header 'Content-Length': the Content-Length header will be generated automatically according to the size of each part";

	switch (type($data)) {
	    case Type::String: {
		my string $ct = $hdr."Content-Type";
		# NOTE we do not check if the charset matches the actual encoding here
		# we just add the charset= text if it's not already present
		if ($ct !~ /charset=/) {
		    if (strlen($ct))
			$ct += "; ";
		    $ct += sprintf("charset=%s", get_encoding($data));
		    $hdr."Content-Type" = $ct;		
		}
		if (!exists $hdr."Content-Transfer-Encoding")
		    $hdr."Content-Transfer-Encoding" = "8bit";
		break;
	    }
	    case Type::Binary: {
		if (!exists $hdr."Content-Transfer-Encoding")
		    $hdr."Content-Transfer-Encoding" = "binary";		
	    }
	}
	return ( "data" : $data,
		 "hdr"  : $hdr );
    }

    splicePart(any $data, hash $hdr) {
	splice $.l, 0, 0, MultiPartMessage::getPart($data, $hdr);
    }
    addPart(any $data, hash $hdr) {
	$.l += MultiPartMessage::getPart($data, $hdr);
    }
    hash getMsgAndHeaders(string $content_type = "") {
        if (!strlen($content_type))
            $content_type = $.ct;

	if (!elements $.l)
	    throw "HTTP-MULTIPART-ERROR", "the message has no parts";

	my binary $msg = binary(MP_DEFAULT_MSG);
	foreach my hash $m in ($.l) {
	    my string $mstr = "\r\n" + $.boundary + "\r\n";
	    foreach my string $k in (keys $m.hdr)
		$mstr += sprintf("%s: %s\r\n", $k, $m.hdr.$k);
	    my binary $bd = binary($m.data);
	    $mstr += sprintf("Content-Length: %d\r\n\r\n", elements $bd);
	    $msg += $mstr;
	    $msg += $bd;
	}
	$msg += "\r\n" + $.boundary + "--";

	$.hdr."Content-Type" = $content_type;
	return ( "hdr"  : $.hdr,
		 "body" : $msg );
    }
    static string getRandomString(int $len) {
	my File $f = new File();
        # throw an exception if /dev/random cannot be opened for reading
        if ($f.open("/dev/random"))
            throw "HTTP-MULTIPART-ERROR", "error opening /dev/random: ", strerror(errno());

	my binary $bin = $f.readBinary($len);
	my string $str;
        while ($len--) {
	    my int $b = $bin[$len] % 62;
	    my string $c;
	    switch ($b) {
		case < 10: { $c = chr(48 + $b); break; }
		case < 36: { $c = chr(55 + $b); break; }
	        default:   { $c = chr(61 + $b); break; }
	    }
	    $str += $c;
	}
	return $str;
    }
}

#! implements a MultiPartRelatedMessage class, a specialization of MultiPartMessage
class MultiPartRelatedMessage inherits MultiPartMessage {
    public {
        string $.startid;
    }
    constructor(string $boundary = MultiPartMessage::MP_QORE_BOUNDARY) : MultiPartMessage(MPT_RELATED, $boundary) {
    }
    splicePart(any $data, string $id, string $content_type, hash $hdr = hash()) {
	if (!strlen($id)) {
	    throw "MULTIPART-RELATED-ERROR", "missing second argument giving the Content-ID identifying the part; this header is required for each part as this is how the parts are related to the main message";
	}
	$.startid = $id;
        MultiPartMessage::$.splicePart($data, $hdr + ("Content-ID" : $id, "Content-Type" : $content_type));
    }
    addPart(any $data, string $id, string $content_type, hash $hdr = hash()) {
	if (!strlen($id)) {
	    throw "MULTIPART-RELATED-ERROR", "missing second argument giving the Content-ID identifying the part; this header is required for each part as this is how the parts are related to the main message";
	}
	if (!exists $.startid)
	    $.startid = $id;
        MultiPartMessage::$.addPart($data, $hdr + ("Content-ID" : $id, "Content-Type" : $content_type));
    }
    hash getMsgAndHeaders() {
	return MultiPartMessage::$.getMsgAndHeaders($.ct + ";start=" + $.startid);
    }
}
