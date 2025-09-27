from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.common.action_chains import ActionChains
from selenium.webdriver.chrome.service import Service as ChromeService
from pymongo import MongoClient
import time
import tempfile
import re

driver=None
chrome_driver_path='/bin/chromedriver'
user_data_dir=tempfile.mkdtemp()
options=webdriver.ChromeOptions()
#options.add_argument('--headless')
options.add_argument('--ignore-certificate-errors')
options.add_argument('--ignore-ssl-errors')
options.add_argument('--disable-blink-features=AutomationControlled')
options.add_argument('--disable-dev-shm-usage')
options.add_argument('--no-sandbox')
options.add_argument(f'--user-data-dir={user_data_dir}')
service=ChromeService(executable_path=chrome_driver_path)
driver=webdriver.Chrome(service=service,options=options)

driver.get('https://www.mvideo.ru/')
time.sleep(4)
driver.execute_script('window.scrollTo(0,document.body.scrollHeight/4)')
time.sleep(6)

carusel=driver.find_elements(By.XPATH,"//mvid-shelf-group")

target_div=None
next_button=None
for elem in carusel:
	try:
		h2=elem.find_element(By.XPATH,"./mvid-switch-shelf-tab-selector/mvid-carousel/div/div/mvid-chip/div/span[contains(text(),'Хиты продаж')]")
		nex=elem.find_element(By.XPATH,"./mvid-carousel/div[2]/button[2]")
		if h2 and nex:
			h2.click()
			next_button=nex
			target_div=elem
			break
	except:
		continue
if target_div==None or next_button==None:
	print('Error not find trend')
	driver.quit()
	exit()

target_div=target_div.find_element(By.XPATH,"./mvid-carousel/div[1]/div/mvid-product-cards-group")
result=[]
for i in range(2):
	names=target_div.find_elements(By.XPATH,"./div[@class='product-mini-card__name ng-star-inserted']")
	prices=target_div.find_elements(By.XPATH,"./div[@class='product-mini-card__price ng-star-inserted']")
	for j in range(len(names)):
		prices[j]=str(re.search(r"(\d[\s\d]*)",prices[j].text).group(1))
		#print(f'{names[j].text} | {prices[j]}')
		new_element=names[j].text,prices[j]
		result.append(new_element)
	next_button.click()
	time.sleep(4)

driver.quit()

client=MongoClient('localhost',27017)
time.sleep(4)
mydb=client['db']
table1=mydb['table1']
for i in range(len(result)):
	elem={'name':result[i][0], 'price':result[i][1]}
	count=0
	for i in table1.find(elem):
		count+=1
		break
	if count!=0:
		continue
	table1.insert_one(elem)

for i in table1.find():
	print(i)

table1.drop()