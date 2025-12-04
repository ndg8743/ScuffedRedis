const net = require('net');

const client = new net.Socket();

setTimeout(() => {
  client.connect(6380, '127.0.0.1', () => {
    console.log('Connected to server');

    // Send PING command
    const command = Buffer.from([
      0x05, // Array
      0x01, 0x00, 0x00, 0x00, // 1 element
      0x04, // Bulk String
      0x04, 0x00, 0x00, 0x00, // 4 bytes
      'P', 'I', 'N', 'G'
    ]);

    client.write(command);
  });
}, 2000);


client.on('data', (data) => {
  console.log('Received:', data.toString());

  const expectedResponse = Buffer.from([
    0x01, // Simple String
    0x04, 0x00, 0x00, 0x00, // 4 bytes
    'P', 'O', 'N', 'G'
  ]);

  if (data.equals(expectedResponse)) {
    console.log('Test passed!');
    client.destroy();
    process.exit(0);
  } else {
    console.error('Test failed!');
    client.destroy();
    process.exit(1);
  }
});

client.on('close', () => {
  console.log('Connection closed');
});

client.on('error', (err) => {
  console.error('Connection error:', err);
  process.exit(1);
});
