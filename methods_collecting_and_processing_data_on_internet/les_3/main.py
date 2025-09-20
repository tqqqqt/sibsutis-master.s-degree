from pymongo import MongoClient
import json

def loadVacansy(data_base, data_table, vacansy):
	collect=data_base[data_table]
	for i in range(len(vacansy)):
		collect.insert_one(vacansy[i])

def searchVacansy(data_base, data_table, price):
	collect=data_base[data_table] #₽,₸,$ 
	for elem in collect.find({'$and':[
		{'price value':'₸'},{'$or':[ 
			{'price min':{'$gt':price}}, 
			{'$and':[{'price min':{'$lte':price}}, {'price max':{'$gte':price}}]} ]} ]}):
		print(f"{elem['name']} | {elem['price min']} | {elem['price max']} | {elem['price value']}")

def addVacansy(data_base, data_table, name, price_min, price_max, price_val, url_v, site):
	collect=data_base[data_table]
	new_elem={'name':name,
			'price min':price_min,
			'price max':price_max,
			'price value':price_val,
			'link':url_v,
			'site':site}
	res=collect.find(new_elem) 
	count=0
	for i in res:
		count+=1
		break
	if count!=0:
		print('Vacansy exsist')
	else:
		collect.insert_one(new_elem)
		print('Vacansy added')


client=MongoClient()
mydb=client['db']
with open("res.json") as file:
	data=json.load(file)

print('Current tables:',mydb.list_collection_names())
loadVacansy(mydb,'table1',data)
print('Current tables:',mydb.list_collection_names())
searchVacansy(mydb,'table1',100)

print('---------------------------------')
addVacansy(mydb,'table1','Junior Frontend Developer (React + Telegram Mini Apps)',60000,80000,'₽','https://novosibirsk.hh.ru/vacancy/125346198?query=%D0%BF%D1%80%D0%BE%D0%B3%D1%80%D0%B0%D0%BC%D0%BC%D0%B8%D1%81%D1%82&hhtmFrom=vacancy_search_list','hh.ru')
addVacansy(mydb,'table1','Front',2000,9000,'R','https://HELLO','HELLO')
addVacansy(mydb,'table1','Front',2000,9000,'R','https://HELLO','HELLO')
print('---------------------------------')

table1=mydb['table1']
table1.drop()