#!/usr/bin/env python3

# guix shell python python-bleak

import asyncio
from bleak import BleakClient, BleakScanner

UART_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
UART_RX_UUID      = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
UART_TX_UUID      = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

BLE_MAX_CHUNK_SIZE = 20  # safe BLE payload size

def ble_rx_handler(sender, data):
    text = data.decode(errors="ignore")
    print(text, end="", flush=True)


async def stdin_loop(client):
    loop = asyncio.get_running_loop()

    while True:
        msg = await loop.run_in_executor(None, input, "")
        data = msg.encode()
        # length-prefixed frame: [2-byte big-endian length][payload]
        frame = len(data).to_bytes(2, "big") + data

        # chunk the frame to the BLE max payload size
        for i in range(0, len(frame), BLE_MAX_CHUNK_SIZE):
            chunk = frame[i:i + BLE_MAX_CHUNK_SIZE]
            await client.write_gatt_char(UART_RX_UUID, chunk)


async def main():
    print("Scanning for micro:bit...")

    devices = await BleakScanner.discover()

    micorbit = None
    for d in devices:
        # print(f"[device] {d}\n")
        if d.name and "micro:bit" in d.name.lower():
            micorbit = d
            break

    if not micorbit:
        print("No micro:bit found")
        return

    print(f"Connecting to {micorbit.name} ({micorbit.address})")

    async with BleakClient(micorbit.address) as ble:
        print("Connected")

        # subscribe to notifications
        await ble.start_notify(UART_TX_UUID, ble_rx_handler)

        # zero-length frame: ask the device to greet (banner + first
        # prompt) once we're subscribed, so the greeting isn't dropped
        await ble.write_gatt_char(UART_RX_UUID, b"\x00\x00")

        print("Ready, printing incoming messages as they arrive.\n")

        # run stdin loop concurrently with BLE
        await stdin_loop(ble)


if __name__ == "__main__":
    asyncio.run(main())
