apiVersion: apps/v1
kind: Deployment
metadata:
  name: node-app-deployment
spec:
  replicas: 1
  selector:
    matchLabels:
      app: node-app
  template:
    metadata:
      labels:
        app: node-app
    spec:
      containers:
        - name: node-app-container
          image: <your-image-name>
          ports:
            - containerPort: 3001
          env:
            - name: ARDUINO_PORT
              value: /dev/ttyACM0
          command: ["node", "server.js"]
          resources:
            requests:
              cpu: "100m"
              memory: "128Mi"
            limits:
              cpu: "500m"
              memory: "512Mi"
---
apiVersion: v1
kind: Service
metadata:
  name: node-app-service
spec:
  selector:
    app: node-app
  ports:
    - name: http
      port: 80
      targetPort: 3001
  type: LoadBalancer
