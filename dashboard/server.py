# ============================================================================
# MODULE INCLUDES & INSTANTIATION
# ============================================================================

from fastapi import FastAPI, WebSocket    # FastAPI: Web framework; WebSocket: Class managing long-lived duplex connections
from fastapi.responses import FileResponse # FileResponse: Sends files (like HTML) asynchronously as HTTP responses
import asyncio                             # Provides infrastructure for writing single-threaded concurrent code using coroutines
import socket                              # Provides access to the low-level BSD socket interface for network communication

# Instantiate the primary web application routing engine
app = FastAPI()

# Global thread-safe in-memory collection to track connected WebSocket clients
clients = set()

# ============================================================================
# BACKGROUND UDP EVENT LISTENER
# ============================================================================

async def udp_listener():
    # Asynchronous worker task that continuously intercepts incoming UDP 
    # telemetry metrics and broadcasts them immediately to active WebSockets.
    
    # SYNTAX: socket.socket(family=AF_INET, type=SOCK_STREAM, proto=0, fileno=None)
    # PARAMETERS & OPTIONS:
    # - family: socket.AF_INET   -> Specifies IPv4 addressing family for the connection
    #           socket.AF_INET6  -> Specifies IPv6 addressing family for the connection
    # - type:   socket.SOCK_DGRAM -> Configures connectionless datagram transmission protocol (UDP)
    #           socket.SOCK_STREAM-> Configures connection-oriented byte stream transmission protocol (TCP)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    # SYNTAX: sock.bind(address)
    # PARAMETERS:
    # - address: ("127.0.0.1", 5001) -> A tuple containing the local interface IP string and target Port integer to claim
    sock.bind(("127.0.0.1", 5001))
    
    # SYNTAX: sock.setblocking(flag)
    # PARAMETERS:
    # - flag: False -> Disables blocking; socket calls return immediately instead of pausing execution thread
    #         True  -> Enables blocking; calls pause thread until data arrives (default system state)
    sock.setblocking(False)

    # SYNTAX: asyncio.get_running_loop()
    # RETURN VALUE:
    # - Returns the running asynchronous event loop running inside the current execution thread context
    loop = asyncio.get_running_loop()

    print("UDP listener started")

    while True:
        # SYNTAX: await loop.sock_recvfrom(sock, bufsize)
        # PARAMETERS:
        # - sock:    sock -> The non-blocking UDP socket object instance to read from
        # - bufsize: 4096 -> The maximum data payload slice size in bytes to read at once
        data, _ = await loop.sock_recvfrom(sock, 4096)

        # Decode raw binary input stream into an readable UTF-8 string format
        message = data.decode()

        print("UDP RECEIVED:", message)

        # Distribute the payload to all dynamically registered clients
        for client in list(clients):
            try:
                # SYNTAX: await client.send_text(data)
                # PARAMETERS:
                # - data: message -> String variable containing the text string to pass down the socket tunnel
                await client.send_text(message)
            except:
                # Safely evict uncommunicative/broken sockets from our routing table map
                clients.discard(client)

# ============================================================================
# APP EVENT LIFECYCLE HOOKS
# ============================================================================

# SYNTAX: @app.on_event(event_type)
# PARAMETERS:
# - event_type: "startup" -> Registers the designated handler to fire once before the server accepts traffic
#               "shutdown"-> Registers the designated handler to fire during standard server shutdown sequence
@app.on_event("startup")
async def startup():
    # SYNTAX: asyncio.create_task(coro, *, name=None)
    # PARAMETERS:
    # - coro: udp_listener() -> The targeted coroutine execution context to schedule on the main loop
    asyncio.create_task(udp_listener())

# ============================================================================
# WEBSOCKET SUBSCRIPTION ROUTE
# ============================================================================

# SYNTAX: @app.websocket(path)
# PARAMETERS:
# - path: "/ws" -> Defines the URI endpoint pattern mapping client handshake protocol upgrade request actions
@app.websocket("/ws")
async def websocket_endpoint(ws: WebSocket):
    # SYNTAX: await ws.accept(subprotocol=None, headers=None)
    # PARAMETERS:
    # - subprotocol: None -> Upgrades connection status from raw HTTP to active persistent WebSocket channel
    await ws.accept()
    
    # Save connection instance reference context into active registry array
    clients.add(ws)

    try:
        while True:
            # SYNTAX: await ws.receive_text()
            # FUNCTION: Keeps the connection reference context alive by intercepting incoming packets from this user channel
            await ws.receive_text()
    except:
        # Tear down tracking states upon client dropouts or line breaks
        clients.remove(ws)

# ============================================================================
# HTTP DASHBOARD VIEW ROUTE
# ============================================================================

# SYNTAX: @app.get(path, response_class=None)
# PARAMETERS:
# - path: "/" -> Default landing resource route matching primary inbound root requests
@app.get("/")
async def root():
    # SYNTAX: FileResponse(path, status_code=200, headers=None, media_type=None)
    # PARAMETERS:
    # - path: "index.html" -> File path layout containing static HTML client-side dashboard code to deliver
    return FileResponse("index.html")