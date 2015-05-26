static void 
listener_cb
(
    struct ubus_context *ctx, 
    struct ubus_event_handler *ev,
    const char *type, 
    struct blob_attr *msg
);

struct uproxy_control uproxy = {
    .listener = {
        .cb = listener_cb,
    };
};

static void 
listener_cb
(
    struct ubus_context *ctx, 
    struct ubus_event_handler *ev,
    const char *type, 
    struct blob_attr *msg
)
{
	char *str;

	str = blobmsg_format_json(msg, true);
	printf("{ \"%s\": %s }\n", type, str);
	free(str);
}

static void
reconnect_timer(struct uloop_timeout *timeout)
{
	static struct uloop_timeout retry = {
		.cb = reconnect_timer,
	};
	int t = 2;

	if (ubus_reconnect(uproxy.ctx, uproxy.path) != 0) {
		uloop_timeout_set(&retry, t * 1000);
		return;
	}

	add_fd();
}

static inline int 
add_subscriber(struct ubus_subscriber *subscriber)
{
	int err = 0;

	err = ubus_register_subscriber(uproxy.ctx, subscriber);
	if (err) {
		debug_everror("Failed to register subscriber(%s)", ubus_strerror(err));
	}

    return 0;
}

static inline int
add_listener(struct ubus_event_handler *listener)
{
    int err;

    err = ubus_register_event_handler(uproxy.ctx, listener, "*");
    if (err) {
        return err;
    }

    return 0;
}

static void
connection_lost(struct ubus_context *ctx)
{
	reconnect_timer(NULL);
}

static inline int
uproxy_ubus_connect(char *path)
{
    uproxy.path = path;
	uproxy.ctx = ubus_connect(path);
	if (NULL==uproxy.ctx) {
	    debug_everror("connect ubus failed");
	    
		return -EIO;
    }
	uproxy.ctx->connection_lost = connection_lost;
	
    ubus_add_uloop(uproxy.ctx);
    os_closexec(uproxy.ctx->sock.fd);

    return 0;
}

int 
uproxy_ubus_init(char *path)
{
	int err;

    uloop_init();
	err = uproxy_ubus_connect(path);
    if (err) {
        return err;
    }
    
    add_subscriber(&uproxy.subscriber);
    add_listener(&uproxy.listener);
    
    debug_evok("ubus init");
    
	return 0;
}

void
uproxy_ubus_fini(void)
{
    if (uproxy.ctx) {
	    ubus_free(uproxy.ctx);
	    uloop_done();
	}

	debug_evok("ubus fini");
}


/******************************************************************************/
