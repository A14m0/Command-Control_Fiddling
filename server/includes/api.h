/* defines the transport API structure */


// defines error codes that can be returned by transport functions

#define API_OK 0
#define API_ERR_GENERIC 1
#define API_ERR_WRITE 2
#define API_ERR_READ 3
#define API_ERR_LISTEN 4
#define API_ERR_BIND 5
#define API_ERR_ACCEPT 6
#define API_ERR_AUTH 7
#define API_ERR_CLIENT 8
#define API_ERR_LOCAL 9


// structures that outlines transports
typedef struct _api_ret {
    int error_code;
    void *data;
} api_return, *papi_return;



//////////////// NOTE: FUNCTIONS TBD ///////////////////////////////////////
typedef struct _transport {
    // start and end functions
    api_return (*init)(void* instance_data);
    api_return (*end)(void* instance_data);


    // gets the next available tasking
    // returns tasking struct with OP_NODATA if nothing available
    api_return (*fetch_tasking)(void* instance_struct);

    // starts listening for this instance
    api_return (*listen)(void* instance_struct);

    // returns transport name info
    const char * (*get_tname)();
    // returns the currently connected agent's name
    api_return (*get_aname)(void* insatnce_struct);
    // returns transport ID
    int (*get_id)();
    // sets the listening port of the instance
    api_return (*set_port)(void* instance_struct, int portno);
} transport_t, *ptransport_t;
////////////////////////////////////////////////////////////////////////////
