/* Channels API.

   You should implement the methods declared here in a file channels.c.
   You may use additional .c and header (.h) files, in which case you also
   need to adapt the makefile.
   
   In all the following, you can assume that the channel number will be in
   the range 0-7 so you can just deal with a fixed number of channels.
*/

/*
 * Must be called exactly once before using any channels.
 * returns 0 on success or < 0 on error, in which case it is not safe
 * to use the channels library.
 */
int ch_setup();

/*
 * Function to clean up before closing the program. After calling this,
 * it is not safe to use any channels functions.
 * returns 0 on success or < 0 on error.
 *
 * Precondtions: ch_setup() was called earlier and no-one is currently
 * listening on any channels.
 */
int ch_destroy();

/*
 * Sends a message on a channel. Each channel has a capacity of one
 * message so if there is no message currently pending in the channel,
 * the send function must not wait for the message to be received.
 * If there is already a message in the channel that has not been delivered,
 * ch_send blocks until the first message has been delivered.
 *
 * channel = the channel number.
 * msg = the message to send. By sending a message, the caller transfers
 * ownership of the message to the channel. Messages must be on the heap.
 * returns 0 on success and < 0 on error.
 *
 * Preconditions: Message cannot be NULL.
 */
int ch_send(int channel, void* msg);

/*
 * Receive a message on a channel. If no message is available,
 * this function blocks until one is received.
 * channel = the channel number.
 * dest = address of a pointer where the message should be stored.
 * This transfers ownership of the message to the receiver.
 * returns 0 on success and < 0 on error.
 * In case of an error, *dest is set to NULL.
 *
 */
int ch_recv(int channel, void** dest);

/*
 * Similar to ch_recv except this one does not block. If no message
 * is available on this channel, it immediately returns with value
 * 0 and *dest set to NULL. If a message was retrieved, returns 1
 * and *dest is set to the message.
 */
int ch_tryrecv(int channel, void** dest);

/*
 * Check if there is currently a message pending in the channel.
 * channel = the channel number.
 * returns 1 if there is currently a pending message, 0 if there is not,
 * -1 in case of errors.
 */
int ch_peek(int channel);
