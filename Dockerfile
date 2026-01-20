FROM python:3.11-slim
WORKDIR /app
ENV PYTHONUNBUFFERED=1
RUN pip install mysql-connector-python
COPY server.py /app/server.py
CMD ["python", "server.py"]
