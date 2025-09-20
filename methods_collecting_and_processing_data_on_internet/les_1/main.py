import requests

req_str="https://api.openweathermap.org/data/2.5/weather?q=Moscow&appid=[api_key]&units=metric"
res=requests.get(req_str)
print(res.text)