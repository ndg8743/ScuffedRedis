import net from 'net';

class SimpleRedisClient {
  private socket: net.Socket | null = null;
  private host: string;
  private port: number;

  constructor(host: string = 'localhost', port: number = 6379) {
    this.host = host;
    this.port = port;
  }

  async connect(): Promise<void> {
    return new Promise((resolve, reject) => {
      this.socket = new net.Socket();
      this.socket.connect(this.port, this.host, () => {
        console.log('Connected to simple Redis server');
        resolve();
      });
      this.socket.on('error', reject);
    });
  }

  private sendCommand(cmd: string): Promise<string> {
    return new Promise((resolve, reject) => {
      if (!this.socket) {
        reject(new Error('Not connected'));
        return;
      }

      // Send command with 4-byte length prefix
      const cmdBuffer = Buffer.from(cmd);
      const lengthBuffer = Buffer.allocUnsafe(4);
      lengthBuffer.writeUInt32LE(cmdBuffer.length, 0);
      
      this.socket.write(Buffer.concat([lengthBuffer, cmdBuffer]));

      // Read response
      this.socket.once('data', (data) => {
        if (data.length < 4) {
          resolve('');
          return;
        }
        
        const responseLength = data.readUInt32LE(0);
        const response = data.slice(4, 4 + responseLength).toString();
        resolve(response);
      });
    });
  }

  async ping(): Promise<string> {
    return await this.sendCommand('PING');
  }

  async get(key: string): Promise<string | null> {
    const result = await this.sendCommand(`GET ${key}`);
    return result === '' ? null : result;
  }

  async set(key: string, value: string): Promise<void> {
    await this.sendCommand(`SET ${key} ${value}`);
  }

  disconnect(): void {
    if (this.socket) {
      this.socket.destroy();
      this.socket = null;
    }
  }
}

export { SimpleRedisClient };
