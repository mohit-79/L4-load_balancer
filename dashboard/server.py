from fastapi import FastAPI, WebSocket
import asyncio
import socket

app = FastAPI()

clients = set()

# async def udp_listener():
#     print("UDP listener started")

#     sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
#     sock.bind(("127.0.0.1", 5001))
#     sock.setblocking(False)

#     loop = asyncio.get_running_loop()

#     while True:
#         data, _ = await loop.sock_recvfrom(sock, 4096)

#         print("Received:", data.decode())

#         for client in list(clients):
#             try:
#                 await client.send_text(data.decode())
#             except:
#                 pass

async def udp_listener():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("127.0.0.1", 5001))
    sock.setblocking(False)

    loop = asyncio.get_running_loop()

    print("UDP listener started")

    while True:
        data, _ = await loop.sock_recvfrom(sock, 4096)

        message = data.decode()

        print("UDP RECEIVED:", message)

        for client in list(clients):
            try:
                await client.send_text(message)
            except:
                clients.discard(client)
@app.on_event("startup")
async def startup():
    asyncio.create_task(udp_listener())

@app.websocket("/ws")
async def websocket_endpoint(ws: WebSocket):
    await ws.accept()
    clients.add(ws)

    try:
        while True:
            await ws.receive_text()
    except:
        clients.remove(ws)

# @app.get("/")
# async def root():
#     return {"status": "dashboard alive"}
from fastapi.responses import FileResponse
@app.get("/")
async def root():
    return FileResponse("index.html")