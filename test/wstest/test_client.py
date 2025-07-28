#-------------------------------------------------------------------------------
# test_client.py
#-------------------------------------------------------------------------------


import asyncio
import websockets


async def test():
	uri = "ws://localhost:3001"
	async with websockets . connect(uri) as websocket:
		print("Connected to server")

		# Send command
		await websocket . send('{"cmd": "get"}')
		print("Sent: get")

		# Send command 2
		await websocket . send('{"cmd": "set", "ch": 2, "val": true}')
		print("Sent: set")

		# Get response
		response = await websocket . recv()
		print("Received:", response)


asyncio . run(test())
