from flask import Flask, request, jsonify
from time import time

app = Flask(__name__)

# Dictionary to store request counts and timestamps
request_data = {}

# Rate limit parameters
RATE_LIMIT = 200
BLOCK_TIME = 60  # in seconds

@app.route('/')
def index():
    ip = request.remote_addr
    current_time = time()

    # Initialize IP data if it doesn't exist
    if ip not in request_data:
        request_data[ip] = {'count': 1, 'timestamp': current_time}
        return jsonify(message="Request successful")

    # Update request count and check rate limit
    request_info = request_data[ip]
    if current_time - request_info['timestamp'] < BLOCK_TIME:
        request_info['count'] += 1
        if request_info['count'] > RATE_LIMIT:
            return jsonify(message="Too many requests. Try again later."), 429
    else:
        # Reset count and timestamp after BLOCK_TIME
        request_data[ip] = {'count': 1, 'timestamp': current_time}

    return jsonify(message="Request successful")

if __name__ == '__main__':
    app.run(debug=True)
