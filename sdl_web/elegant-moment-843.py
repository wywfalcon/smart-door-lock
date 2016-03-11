###################################################################################################
#
#	Smart Door Lock Server
#	Author: Wai Yin Wong (Falcon)
#	Description:
#		The server allows the owner to log-in, log-out, register an account with a smart door lock,
#	rename first/last names of users, change access times, set Wi-Fi connection, and enroll / delete
#	fingerprint profiles from the lock
#
###################################################################################################

###################################################################################################
#	Imports
###################################################################################################

import webapp2
import smtplib
import uuid
import random
import datetime
try: import simplejson as json
except: import json
from random import randint
from google.appengine.ext import ndb
from google.appengine.api import mail
from google.appengine.api import memcache

###################################################################################################
#	Initialization
###################################################################################################

# Database entity
user_key = ndb.Key('User', 'default_user')

###################################################################################################
#	Data Structure
###################################################################################################

class User (ndb.Model):
	email = ndb.StringProperty()
	password = ndb.StringProperty()
	fingerID = ndb.IntegerProperty()
	admin = ndb.StringProperty()
	accessSchedule = ndb.StringProperty()
	templates = ndb.StringProperty()
	firstName = ndb.StringProperty()
	lastName = ndb.StringProperty()
	ssid = ndb.StringProperty()
	networkPW = ndb.StringProperty()
	productID = ndb.StringProperty()
	token = ndb.StringProperty()
	
class Product (ndb.Model):
	productID = ndb.StringProperty()
	schedule = ndb.StringProperty()
	deleted = ndb.StringProperty()
	toAdd = ndb.StringProperty()

###################################################################################################
#	User Account Pages
###################################################################################################

class HomePage (webapp2.RequestHandler):
  def get(self):
    self.response.out.write("""
		<!DOCTYPE html>
		<html lang="en">
			<head>
				<meta charset="utf-8">
				<meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1">
				<script type="text/javascript" src="app/js/lib/jquery-1.11.2.min.js"></script>
				<script type="text/javascript" src="app/js/lib/jquery.cookie.js"></script>
				<script type="text/javascript" src="app/js/src/home.js"></script>
				<link rel="stylesheet" type="text/css" href="app/css/src/web.css">
				<link rel="stylesheet" type="text/css" href="app/css/lib/bootstrap.min.css">
				<title>Home</title>
			</head>
			<body>
				<div><img class="bgi" src="app/css/src/img/bgi.jpg"></div>
				<div class="sidebar">
					<img class="avatar" src="{0}">
					<div class="divider"></div>
					<div class="item" id="login">Login</div>
					<div class="item" id="register">Register</div>
					<div class="item" id="forgotPassword">Forgot Password</div>
					<div class="divider"></div>
					<form class="center padding" id="loginForm" action="/account" method="post">
						<div class="label">Email</div><div class="alignCenter"><input type="text" name="email" placeholder="Email"><div class="error"></div></div>
						<div class="label">Password</div><div class="alignCenter"><input type="password" name="password" placeholder="Password"><div class="error"></div></div>
						<input id="login" type="button" class="alignRight button margin" value="Submit"/>
					</form>
					<form class="center padding" id="registerForm" action="/register" method="post">
						<div class="label">First Name</div><div class="alignCenter"><input type="text" name="firstName" placeholder="John"><div class="error"></div></div>
						<div class="label">Last Name</div><div class="alignCenter"><input type="text" name="lastName" placeholder="Do"><div class="error"></div></div>
						<div class="label">Email</div><div class="alignCenter"><input type="text" name="email" placeholder="johndo@email.com"><div class="error"></div></div>
						<div class="label">Password</div><div class="alignCenter"><input type="password" name="password" placeholder="enter password"><div class="error"></div></div>
						<div class="label">Confirm Password</div><div class="alignCenter"><input type="password" name="passwordConfirm" placeholder="enter password again"><div class="error"></div></div>
						<div class="label">Product ID</div><div class="alignCenter"><input type="text" name="productID" placeholder="123.123.123.123"><div class="error"></div></div>
						<input id="register" type="button" class="alignRight button margin" value="Submit"/>
					</form>
					<form class="center padding" id="forgotPasswordForm" action="/forgotPassword" method="post">
						<div class="label">Email</div><div class="alignCenter"><input type="text" name="email" placeholder="Email"><div class="error"></div></div>
						<input id="forgotPassword" type="button" class="alignRight button margin" value="Submit"/>
					</form>
				</div>
				<div class="mainPage">
					<div class="topMenu"></div>
					<table class="pageContainer">
						<tr>
							<td class="subPageContainer padding">
								<h1>(Gallery to be inserted)</h1>
							</td>
							<td class="subPageContainer pageContent padding">
								<h1>SDP: Smart Door Lock</h1>
								<h3>Convenience without sacrificing security</h3>
								<p>Our lock uses a fingerprint reader to authenticate the user. You can go in and out even without your keys or smart-phone</p>
								<h3>Never get locked out again</h3>
								<p>The lock has a solar panel and power generation so power outage will not lock you out.</p>
								<h3>Elaborate Software Interface</h3>
								<p>There is no need to pick up the manual because everything just makes sense from the start.</p>
							</td>
						</tr>
					</table>
				</div>
			</body>
		</html>
	""".format('app/css/src/img/logo.png'))
		
class Account (webapp2.RequestHandler):
	def post(self):
		### Get user who logged in
		email = self.request.get("email")
		password = self.request.get("password")
		admin = ndb.gql('SELECT * FROM User WHERE email=:1', email).get()
		label = 'success'
		message = 'You will be redirected momentarilly'
		
		if admin is None:
			label = 'userNotFound'
			message = 'user not found'
		elif password != admin.password:
			label = 'incorrectPW'
			message = 'password is incorrect'
		else:
			users = ndb.gql('SELECT * FROM User WHERE admin=:1 ORDER BY email', admin.admin).fetch()
			rootAdmin = ndb.gql('SELECT * FROM User WHERE email=:1', admin.admin).get()
			htmlString = """
				<tr class="bold">
					<td class="column"></td>
					<td class="column idColumn">ID</td>
					<td class="column emailColumn">Email</td>
					<td class="column firstnameColumn">First Name</td>
					<td class="column lastnameColumn">Last Name</td>
					<td class="column" colspan="2">Schedule</td>
				</tr>
				"""
			htmlString += """
				  <tr class="edit">
					  <td class="column hideShow hideUsers alignCenter border">&#9650</td>
					  <td class="column idColumn">{0}</td>
					  <td class="column emailColumn">{1}</td>
					  <td class="column firstnameColumn">{2}</td>
					  <td class="column lastnameColumn">{3}</td>
					  <td class="column" hidden>{4}</td>
					  <td class="column scheduleColumn"></td>
					  <td class="column" hidden>{5}</td>
				  </tr>
			  """.format(str(rootAdmin.fingerID), rootAdmin.email, rootAdmin.firstName, rootAdmin.lastName, rootAdmin.accessSchedule.encode('ascii', 'ignore'), rootAdmin.admin)
			for user in users:
				if user.admin != user.email:
					htmlString += """
									  <tr class="edit">
										  <td class="column"></td>
										  <td class="column idColumn">{0}</td>
										  <td class="column emailColumn">{1}</td>
										  <td class="column firstnameColumn">{2}</td>
										  <td class="column lastnameColumn">{3}</td>
										  <td class="column" hidden>{4}</td>
										  <td class="column scheduleColumn"></td>
										  <td class="column" hidden>{5}</td>
									  </tr>
								  """.format(str(user.fingerID), user.email, user.firstName, user.lastName, user.accessSchedule.encode('ascii', 'ignore'), user.admin)
			self.response.out.write("""
				<!DOCTYPE html>
				<html lang="en">
					<head>
						<meta charset="utf-8">
						<meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1">
						<script type="text/javascript" src="app/js/lib/jquery-1.11.2.min.js"></script>
						<script type="text/javascript" src="app/js/lib/jquery.cookie.js"></script>
						<script type="text/javascript" src="app/js/src/account.js"></script>
						<link rel="stylesheet" type="text/css" href="app/css/src/web.css">
						<link rel="stylesheet" type="text/css" href="app/css/lib/bootstrap.min.css">
						<title>Account</title>
					</head>
					<body>
						<div><img class="bgi" src="app/css/src/img/bgi.jpg"></div>
						<div class="sidebar">
							<img class="avatar" src="{3}">
							<div class="welcome">{0}</div>
							<div class="hiddenInfo" hidden>
								<div>{4}</div>
							</div>
							<div class="item" id="notification">New Enrolls<span id="newMsg" class="alignRight">0</span></div>
							<div class="divider"></div>
							<div class="item" id="addNewUser">Add New User</div>
							<div class="item" id="setWifi">Set Wi-Fi</div>
							<div class="divider"></div>
							<div class="item" id="deleteAccount">Delete Account</div>
							<div class="item" id="resetPW">Reset Password</div>
							<div class="item" id="logout">Logout</div>
							<div class="divider"></div>
						</div>
						<div class="mainPage">
							<div class="topMenu">
								<input class="searchBar" placeholder="search user" />
							</div>
							<table class="pageContainer">
								<tr>
									<td class="subPageContainer"><table class="stretch">{1}</table></td>
									<td class="subPageContainer pageContent"></td>
								</tr>
							</table>
						</div>
						<script>$.cookie('ssid','{2}');</script>
					</body>
				</html>
			""".format(email, htmlString, admin.ssid, 'app/css/src/img/avatar.gif', admin.templates))
			return
		obj = {
				'action': 'account',
				'label': label, 'msg': message
			  }
		self.response.out.write(json.dumps(obj))
		
class Expired (webapp2.RequestHandler):
	def get(self):
		title = 'Error'
		msg = self.request.get("msg")
		if msg == 'accountDelete':
			title = 'Account Deleted'
			msg = "You have deleted your account along with all other user accounts under you. Thank you for using our service!"
		elif msg == 'expired':
			title = 'Login Expired'
			msg = 'Your login has expired'
		else:
			msg = 'An error has occured. Please login again.'
		self.response.out.write(
			"""
				<head>
					<meta charset="utf-8">
					<meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1">
					<script type="text/javascript" src="app/js/lib/jquery-1.11.2.min.js"></script>
					<script type="text/javascript" src="app/js/lib/jquery.cookie.js"></script>
					<link rel="stylesheet" type="text/css" href="app/css/src/web.css">
					<title>Expired</title>
				</head>
				<body><table class="center boundBox whiteFont">
					<div><img class="bgi" src="app/css/src/img/bgi.jpg"></div>
					<tr><td><h1 class="padding border alignCenter">{0}</h1></td></tr>
					<tr><td class="padding"><p><div class="padding">{1}</p></div></td></tr>
					<tr><td><form name="redirect" action="/" method="get">
						<input class="alignRight" type="submit" value="Dismiss"/>
					</form></td></tr>
				</table></body>
			""".format(title, msg)
		)
		
class Register (webapp2.RequestHandler):
	def post(self):
		### Gather user information
		user = User(parent=user_key)
		user.email = self.request.get("email")
		user.password = self.request.get("password")
		user.ssid = self.request.get("ssid")
		user.networkPW = self.request.get("networkPW")
		user.productID = self.request.get("productID")
		user.admin = user.email
		
		### Set default names
		user.firstName = self.request.get("firstName")
		user.lastName = self.request.get("lastName")
		user.accessSchedule = ""
		user.fingerID = random.randint(0, 199)
		
		label = 'success'
		message = 'Thanks for registering. You will be redirected momentarilly.'
		
		### Check if user name taken
		admin = ndb.gql('SELECT * FROM User WHERE email=:1', user.email).get()
		if admin is not None:
			label = 'email'
			message = 'email is taken'
		else:
			user.put()
			product = Product(parent=user_key)
			product.productID = user.productID
			product.put()
			
		obj = {
				'action': 'register',
				'label': label, 'msg': message
			  }
		self.response.out.write(json.dumps(obj))

class DeleteAccount (webapp2.RequestHandler):
	def post(self):
		email = self.request.get("email")
		password = self.request.get("password")
		admin = ndb.gql('SELECT * FROM User WHERE email=:1', email).get()
		if password != admin.password:
			obj = {
				'action': 'deleteAccount',
				'label': 'incorrectPW', 'msg': 'password is incorrect'
			  }
			self.response.out.write(json.dumps(obj))
		else:
			users = ndb.gql('SELECT * FROM User WHERE admin=:1', admin.email).fetch()
			message = "{0} and all associated user accounts have been removed. Thank you for using our service!".format(admin.email)
			for user in users:
				user.key.delete()
			
			obj = {
				'action': 'deleteAccount',
				'label': 'success', 'msg': message
			  }
			self.response.out.write(json.dumps(obj))
		
###################################################################################################
#	Communication with the Smart Door Lock
###################################################################################################		
	
class Enroll (webapp2.RequestHandler):
	def post(self):
		user = User(parent=user_key)
		admin = ndb.gql('SELECT * FROM User WHERE email=:1', self.request.get("admin")).get()
		label = 'Fail'
		message = 'You are not authorized to add users';
		if admin.admin == admin.email:
			user.admin = self.request.get("admin")
			user.email = self.request.get("email")
			user.password = uuid.uuid4().hex
			user.firstName = self.request.get("firstName")
			user.lastName = self.request.get("lastName")
			user.fingerID = random.randint(0, 199)
			user.accessSchedule = self.request.get("schedule")
			
			### Check if email is taken for this particular admin
			duplicate = ndb.gql('SELECT * FROM User WHERE admin=:1 AND email=:2', admin.email, user.email).get()
			addingAdmin = ndb.gql('SELECT * FROM User WHERE email=:1', user.email).get()
			if duplicate is not None:
				message = 'Email has already been registered'
			elif addingAdmin is not None and addingAdmin.admin == addingAdmin.email:
				message = 'Cannot add the user because this user is an admin'
			else:
				user.token = uuid.uuid4().hex
				memcache.set(key=user.token, value=1, time=86400)
				emailContent = mail.EmailMessage(sender="SDL Support <waywong@ucsc.edu>",
											subject="Smart Door Lock: Enrollment Invitation")
				emailContent.to = user.firstName + " " + user.lastName + "<" + user.email + ">"
				emailContent.html = """
					<html>
						<body>
							<h3>Smart Door Lock: Enrollment Invitation</h3>
							<p>Hello {0},</p>
							<p>You are invited by {1} to gain access to her security system. Please follow these steps to enroll your fingerprint:</p>
							<ol>
								<li>Arrive at <u>{2}</u></li>
								<li>Connect to the <b>SDL Network</b> on your mobile device's Wi-Fi settings</li>
								<li>Press the button behind the handle</li>
								<li>Click <a href="http://{3}/?tok={4}">here</a></u></li>
							</ol>
							<h3>SDL Support Team</h3>
						</body>
					</html>
				""".format(user.firstName, user.admin, self.request.get("location"), admin.productID, user.token)
				emailContent.send()
				label = 'Success'
				message = 'You have email invited <b>{0}</b> to enroll. This email will expire in one day. Please make sure the user is enrolled before the expiry.'.format(user.email);
				user.put()
		obj = {
				'action': 'enroll',
				'label': label, 'msg': message,
				'admin': user.admin,
				'html': """
						  <tr class="edit">
							  <td class="column"></td>
							  <td class="column idColumn">{0}</td>
							  <td class="column emailColumn">{1}</td>
							  <td class="column firstnameColumn">{2}</td>
							  <td class="column lastnameColumn">{3}</td>
							  <td class="column" hidden>{4}</td>
							  <td class="column scheduleColumn"></td>
							  <td class="column" hidden>{5}</td>
						  </tr>
					  """.format(str(user.fingerID), user.email, user.firstName, user.lastName, user.accessSchedule, user.admin)
			  }
		self.response.out.write(json.dumps(obj))
	
class Delete (webapp2.RequestHandler):
	def post(self):
		user = ndb.gql('SELECT * FROM User WHERE email=:1 limit 1', self.request.get("email")).get()
		admin = ndb.gql('SELECT * FROM User WHERE email=:1 limit 1', self.request.get("admin")).get()
		label = 'Fail'
		message = 'No permission to delete';
		if user.admin == admin.email:
			if admin.email == admin.admin == user.email:
				message = 'Please select <b>Delete Account</b> from the left menu to remove your account'
			elif user.email == user.admin:
				message = 'Cannot delete another admin'
			else:
				product = ndb.gql('SELECT * FROM Product WHERE productID=:1', admin.productID).get()
				if product.deleted == "":
					product.deleted = str(user.fingerID)
				else:
					product.deleted = product.deleted + ',' + str(user.fingerID)
				product.put()
				label = 'Success'
				message = 'User <b>{0}</b> is deleted'.format(user.email)
				user.key.delete()
		obj = {
				'action': 'delete',
				'label': label, 'msg': message
			  }
		self.response.out.write(json.dumps(obj))
		
class SaveTemplate (webapp2.RequestHandler):
	def post(self):
		admin = ndb.gql('SELECT * FROM User WHERE email=:1 limit 1', self.request.get("admin")).get()
		schedule = self.request.get("schedule")
		name = self.request.get("name")
		label = 'Fail'
		message = 'Please enter a name for your schedule template'
		if admin is not None and name != '':
			if admin.templates is None:
				admin.templates = "{0},{1}".format(name, schedule)
			else:
				admin.templates += ";{0},{1}".format(name, schedule)
			label = 'Success'
			message = name + ' is saved'
			admin.put()
		obj = {
				'action': 'saveTemplate',
				'label': label, 'msg': message,
				'name': name, 'value': schedule
			  }
		self.response.out.write(json.dumps(obj))

###################################################################################################
#	Behind the scene Wi-Fi communication
###################################################################################################

class Admin (webapp2.RequestHandler):
	def get(self):
		productID = self.request.get("productID")
		mcuAdd = self.request.get("add")
		mcuDel = self.request.get("del")
		product = ndb.gql('SELECT * FROM Product WHERE productID=:1', productID).get()
		schedule = ""
		deleted = ""
		if product is not None:
			schedule = product.schedule
			deleted = product.deleted
			if product.toAdd == "":
				product.toAdd = mcuAdd
			else:
				product.toAdd = product.toAdd +','+ mcuAdd
				product.toAdd = product.toAdd
			product.put()
			idToDelete = str(mcuDel).split(',')
			for id in idToDelete:
				if id == '':
					continue
				user = ndb.gql('SELECT * FROM User WHERE fingerID=:1 limit 1', int(id)).get()
				if user is not None:
					user.key.delete()
			memcache.set(key=productID, value=product.toAdd+"&"+mcuDel, time=86400)
			self.response.out.write(
				"""productID={2}&day={0}&time={1}&del={3}&sch={4}$""".format(
						(datetime.datetime.today().weekday()+1)%6,
						datetime.datetime.now().time(),
						productID,
						deleted,
						schedule
					)
			)
		else:
			self.response.out.write('productID not found')
	def post(self):
		productID = self.request.get("productID")
		mcuAdd = self.request.get("add")
		mcuDel = self.request.get("del")
		product = ndb.gql('SELECT * FROM Product WHERE productID=:1', productID).get()
		schedule = ""
		deleted = ""
		if product is not None:
			schedule = product.schedule
			deleted = product.deleted
			if product.toAdd == "":
				product.toAdd = mcuAdd
			else:
				product.toAdd = product.toAdd +','+ mcuAdd
				product.toAdd = product.toAdd
			product.put()
			idToDelete = str(mcuDel).split(',')
			for id in idToDelete:
				if id == '':
					continue
				user = ndb.gql('SELECT * FROM User WHERE fingerID=:1 limit 1', int(id)).get()
				if user is not None:
					user.key.delete()
			memcache.set(key=productID, value=product.toAdd+"&"+mcuDel, time=86400)
			self.response.out.write(
				"""productID={2}&day={0}&time={1}&del={3}&sch={4}$""".format(
						(datetime.datetime.today().weekday()+1)%6,
						datetime.datetime.now().time(),
						productID,
						deleted,
						schedule
					)
			)
		else:
			self.response.out.write('productID not found')
			
class Update (webapp2.RequestHandler):
	def post(self):
		email = self.request.get("email")
		productID = memcache.get(email)
		if productID is None:
			user = ndb.gql('SELECT * FROM User WHERE email=:1', email).get()
			productID = user.productID
			memcache.set(key=email, value=productID, time=604800)
		update = memcache.get(productID)
		self.response.out.write(update)
		
class ClearNotification (webapp2.RequestHandler):
	def post(self):
		admin = ndb.gql('SELECT * FROM User WHERE email=:1 limit 1', self.request.get("admin")).get()
		productID = memcache.get(admin.email)
		if productID is None:
			user = ndb.gql('SELECT * FROM User WHERE email=:1', admin.email).get()
			productID = user.productID
			memcache.set(key=admin.email, value=productID, time=604800)
		product = ndb.gql('SELECT * FROM Product WHERE productID=:1', productID).get()
		product.toAdd = self.request.get("toAdd")
		memcache.set(key=productID, value=product.toAdd, time=604800)
		product.put()
		user = User(parent=user_key)
		user.admin = admin.email
		user.email = self.request.get("email")
		user.password = uuid.uuid4().hex
		user.firstName = self.request.get("firstName")
		user.lastName = self.request.get("lastName")
		user.fingerID = int(self.request.get("id"))
		user.accessSchedule = "full"
		user.token = uuid.uuid4().hex
		memcache.set(key=user.token, value=1, time=86400)
		emailContent = mail.EmailMessage(sender="SDL Support <waywong@ucsc.edu>",
									subject="Smart Door Lock: Enrollment Invitation")
		emailContent.to = user.firstName + " " + user.lastName + "<" + user.email + ">"
		emailContent.html = """
			<html>
				<body>
					<h3>Smart Door Lock: Enrollment Invitation</h3>
					<p>Hello {0},</p>
					<p>You are invited by {1} to gain access to her security system. Please follow these steps to enroll your fingerprint:</p>
					<ol>
						<li>Arrive at <u>{2}</u></li>
						<li>Connect to the <b>SDL Network</b> on your mobile device's Wi-Fi settings</li>
						<li>Press the button behind the handle</li>
						<li>Click <a href="http://{3}/?tok={4}">here</a></u></li>
					</ol>
					<h3>SDL Support Team</h3>
				</body>
			</html>
		""".format(user.firstName, user.admin, self.request.get("location"), admin.productID, user.token)
		emailContent.send()
		label = 'Success'
		message = 'You have email invited <b>{0}</b> to enroll. This email will expire in one day. Please make sure the user is enrolled before the expiry.'.format(user.email);
		user.put()
		obj = {
				'action': 'enroll',
				'label': label, 'msg': message,
				'admin': user.admin,
				'html': """
						  <tr class="edit">
							  <td class="column"></td>
							  <td class="column idColumn">{0}</td>
							  <td class="column emailColumn">{1}</td>
							  <td class="column firstnameColumn">{2}</td>
							  <td class="column lastnameColumn">{3}</td>
							  <td class="column" hidden>{4}</td>
							  <td class="column scheduleColumn"></td>
							  <td class="column" hidden>{5}</td>
						  </tr>
					  """.format(str(user.fingerID), user.email, user.firstName, user.lastName, user.accessSchedule, user.admin)
			  }
		self.response.out.write(json.dumps(obj))
		
###################################################################################################
#	Forgot and Reset Password Pages
###################################################################################################

class ForgotPassword (webapp2.RequestHandler):
	def post(self):
		email = self.request.get("email")
		user = ndb.gql('SELECT * FROM User WHERE email=:1', email).get()
		label = 'success'
		message = 'Please follow the link in your email to reset your psasword.'
		if user is None:
			label = 'userNotFound'
			message = 'User not found'
		else:
			user.token = uuid.uuid4().hex
			memcache.set(key=email, value=user.token, time=300)
			emailContent = mail.EmailMessage(sender="SDL Support <waywong@ucsc.edu>",
										subject="SDL - Reset Password")
			emailContent.to = user.firstName + " " + user.lastName + "<" + email + ">"
			emailContent.html = """
				<html>
					<body>
						<h3>Password Reset</h3>
						<p>Hello {0},</p>
						<p>Please reset password by clicking <a href="http://elegant-moment-843.appspot.com/resetPassword?tok={1}">here</a>.</p>
						<h3>SDL Support Team</h3>
					</body>
				</html>
			""".format(user.firstName, user.token)
			emailContent.send()
			user.put()
		obj = {
				'action': 'forgotPassword',
				'label': label, 'msg': message
			  }
		self.response.out.write(json.dumps(obj))
		
class ResetPassword (webapp2.RequestHandler):
	def get(self):
		sqTok = self.request.get("tok")
		user = ndb.gql('SELECT * FROM User WHERE token=:1', sqTok).get()
		memTok = memcache.get(user.email)
		if user is None or memTok != sqTok or memTok != sqTok:
			self.redirect('/expired?msg=token');
		else:
			self.response.out.write(
				"""
					<head>
						<meta charset="utf-8">
						<meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1">
						<script type="text/javascript" src="app/js/lib/jquery-1.11.2.min.js"></script>
						<script type="text/javascript" src="app/js/lib/jquery.cookie.js"></script>
						<link rel="stylesheet" type="text/css" href="app/css/src/web.css">
						<title>Expired</title>
					</head>
					<body><table class="center boundBox whiteFont">
						<div><img class="bgi" src="app/css/src/img/bgi.jpg"></div>
						<tr><td><h1 class="padding border alignCenter">Reset Password</h1></td></tr>
						<tr><td><form name="redirect" action="/savePassword?tok={0}" method="post">
							<div class="label">New Password</div><div class="alignCenter margin"><input type="password" name="password" placeholder="enter password"><div class="error"></div></div>
							<div class="label">Confirm Password</div><div class="alignCenter margin"><input type="password" name="passwordConfirm" placeholder="enter password again"><div class="error"></div></div>
							<input class="alignRight" type="submit" value="Confirm"/>
						</form></td></tr>
					</table></body>
				""".format(sqTok)
			)
		
class SavePassword (webapp2.RequestHandler):
	def post(self):
		sqTok = self.request.get("tok")
		user = ndb.gql('SELECT * FROM User WHERE token=:1', sqTok).get()
		memTok = memcache.get(user.email)
		if user is None or memTok != sqTok or memTok != sqTok:
			self.redirect('/expired?msg=token');
		else:
			user.password = self.request.get("password")
			user.put()
			self.response.out.write(
				"""
					<head>
						<meta charset="utf-8">
						<meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1">
						<script type="text/javascript" src="app/js/lib/jquery-1.11.2.min.js"></script>
						<script type="text/javascript" src="app/js/lib/jquery.cookie.js"></script>
						<link rel="stylesheet" type="text/css" href="app/css/src/web.css">
						<title>Expired</title>
					</head>
					<body><table class="center boundBox whiteFont">
						<div><img class="bgi" src="app/css/src/img/bgi.jpg"></div>
						<tr><td><h1 class="padding border alignCenter">Message</h1></td></tr>
						<tr><td class="padding"><p><div class="padding">Your have reset your password.</p></div></td></tr>
						<tr><td><form name="redirect" action="/account" method="post">
							<input hidden type="password" name="email" value="{0}"/>
							<input hidden type="password" name="password" value="{1}"/>
							<input class="alignRight" type="submit" value="Dismiss"/>
						</form></td></tr>
					</table></body>
				""".format(user.email, user.password)
			)

###################################################################################################
#	Set User Information
###################################################################################################

class SetWifi (webapp2.RequestHandler):
	def post(self):
		admin = ndb.gql('SELECT * FROM User WHERE email=:1', self.request.get("email")).get()
		label = 'Fail'
		message = 'You are not authorized to change the connection'
		if admin.admin == admin.email:
			label = 'Success'
			message = 'Wi-Fi connection was changed to '+admin.ssid
			admin.ssid = self.request.get("ssid")
			admin.networkPW = self.request.get("networkPW")
			admin.put()
		obj = {
			'action': 'setWifi',
			'label': label, 'msg': message,
			'ssid': admin.ssid
		  }
		self.response.out.write(json.dumps(obj))

class SetPassword (webapp2.RequestHandler):
	def post(self):
		email = self.request.get("email")
		admin = ndb.gql('SELECT * FROM User WHERE email=:1', email).get()
		label = 'Success'
		message = 'You have just reset your password'
		if self.request.get('oldPW') != admin.password:
			label = 'Fail'
			message = 'The old password you have entered is incorrect'
		else:
			admin.password = self.request.get('newPW')
			admin.put()
		obj = {
			'action': 'setPassword',
			'label': label, 'msg': message
		  }
		self.response.out.write(json.dumps(obj))

class Rename (webapp2.RequestHandler):
	def post(self):
		user = ndb.gql('SELECT * FROM User WHERE email=:1 limit 1', self.request.get("email")).get()
		admin = ndb.gql('SELECT * FROM User WHERE email=:1 limit 1', user.admin).get()
		label = 'Fail'
		message = 'Cannot rename other users'
		if admin.email == user.admin:
			user.firstName = self.request.get("firstName")
			user.lastName = self.request.get("lastName")
			label = 'Success'
			message = 'This user has been renamed ' + user.firstName + ' ' + user.lastName
			user.put()
		obj = {
				'action': 'rename',
				'label': label, 'msg': message,
				'firstName': user.firstName,
				'lastName': user.lastName
			  }
		self.response.out.write(json.dumps(obj))
		
class Schedule (webapp2.RequestHandler):
	def post(self):
		user = ndb.gql('SELECT * FROM User WHERE email=:1 limit 1', self.request.get("email")).get()
		admin = ndb.gql('SELECT * FROM User WHERE email=:1 limit 1', self.request.get("admin")).get()
		label = 'Fail'
		message = 'No permission to delete'
		done = None
		if admin.admin == admin.email:
			schedule = self.request.get("schedule")
			weekYear = self.request.get("weekYear")
			if weekYear == "":
				message = 'Please specify a week to schedule'
			else:
				weekYear = weekYear.split('-W')
				week = str(weekYear[1])
				year = str(weekYear[0])
				today = datetime.date.today()
				thisWeek = str(today.isocalendar()[1])
				thisYear = str(today.year)
				if week == thisWeek and year == thisYear:
					product = ndb.gql('SELECT * FROM Product WHERE productID=:1', admin.productID).get()
					if product.schedule == "":
						product.schedule = "{0},{1}".format(str(user.fingerID), schedule)
					else:
						list = product.schedule.split(';')
						i = 0
						for data in list:
							if str(user.fingerID) == data.split(',')[0]:
								del list[i]
								break
							i += 1
						product.schedule = ';'.join(list) + ";{0},{1}".format(str(user.fingerID), schedule)
					product.put()
				else:
					if user.accessSchedule is None or user.accessSchedule == "":
						accessSchedule = ["{0},{1},{2}".format(year, week, schedule)]
					else:
						accessSchedule = user.accessSchedule.split(';')
						i = 0
						for data in accessSchedule:
							if not data:
								break
							weekSchedule = data.split(',')
							if weekSchedule[0] == year and weekSchedule[1] == week:
								del accessSchedule[i]
								break
							i += 1
						accessSchedule.append("{0},{1},{2}".format(year, week, schedule))
					user.accessSchedule = ';'.join(accessSchedule)
					user.put()
				label = 'Success'
				message = 'User <b>{0}</b> has been rescheduled for Week {1}, {2}. It will take approximately 30 minutes for this change to take effect'.format(user.email, week, year)
		else:
			message = 'No permission to schedule'
		obj = {
				'action': 'schedule',
				'label': label, 'msg': message,
				'schedule': user.accessSchedule
			  }
		self.response.out.write(json.dumps(obj))
		
###################################################################################################
#	Page Request Handler
###################################################################################################

app = webapp2.WSGIApplication([
  ('/', HomePage),
  ('/account', Account),
  ('/register', Register),
  ('/deleteAccount', DeleteAccount),
  ('/enroll', Enroll),
  ('/delete', Delete),
  ('/schedule', Schedule),
  ('/saveTemplate', SaveTemplate),
  ('/rename', Rename),
  ('/expired', Expired),
  ('/admin', Admin),
  ('/update', Update),
  ('/clearNotification', ClearNotification),
  ('/forgotPassword', ForgotPassword),
  ('/setPassword', SetPassword),
  ('/resetPassword', ResetPassword),
  ('/savePassword', SavePassword),
  ('/setWifi', SetWifi)
], debug=True)
