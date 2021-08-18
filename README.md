# super-glue
`super-glue` is designed to be a simple and easy to use piece of software that
provides a layer for gluing apps together. 

Using a simple declarative language, a user of `super-glue` can tell it how to
react when it recieves HTTP/S requests at various endpoints. Our app is
designed to make common tasks (token based authentication, rate limiting,
communicating with other processes through pipes) as straight forward as
possible. Here's an example of what super-glue could be good for:
- You have a Minecraft plugin running in a JVM, and you want players to be able
to send chat messages from a browser.
- One option would be implementing an entire web server in Java, or using a 3rd
party HTTP server. This would consume JVM resources and potentially slow down
the game server. This is also non-flexible; if you wanted players to also be
able to send messages on discord and have them picked up by a bot and sent to
the  server, you'd have to rewrite your plugin or maintain the overhead of
running an HTTP server in the same JVM as the game server.
- Using `super-glue`, this process is simple. The Java app reads from a UNIX 
pipe, and `super-glue` processes incoming HTTP POST requests, and writes the 
chat messages to the pipe. This works equally well for a discord bot; whenever
a message is sent in a channel like `#minecraft-chat`, the bot makes a simple
POST request (for example using JS's fetch API), et voila. 

`super-glue` is currently in a pre-alpha state. The details of the
configuration language have yet to be hammered out. This said, the emphasis
will be on sane defaults and keeping simple things simple.
