$( document ).ready(function() {
	
	var FULL_ACCESS = 'ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ',
		d = new Date();
	
	/************************************************************************************
		Check for expiry
	*************************************************************************************/
	if (!$.cookie("email")) {
		var cookies = $.cookie();
		for (var cookie in cookies) $.removeCookie(cookie);
		window.location.replace('/expired?msg=expired');
	}
	
	/************************************************************************************
		Util Variables
	*************************************************************************************/
	
	var daysOfWeek = ['SUN', 'MON', 'TUE', 'WED', 'THU', 'FRI', 'SAT'];
	
	/************************************************************************************
		Mobile Detect
	*************************************************************************************/
	if (/Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent) ) {
		$('.boundBox').css('width', '100%');
		$('*').css('font-size', '115%')
	} else {
		$('.boundBox').css('width', '60%');
	}
	
	/************************************************************************************
		Util Functions
	*************************************************************************************/
	
	var	enrollUser = function () {
			$('.pageContent').html(
				'<div class="margin">'+
					'<div class="message" hidden></div>'+
					'<h1>Enroll new user</h1>'+
					'<div class="padding">'+
						'<form class="center" id="addNewUserForm" action="/enroll" method="post">'+
							'<div class="padding">First Name</div><div class="alignCenter"><input type="text" name="firstName" placeholder="John"><div class="error"></div></div>'+
							'<div class="padding">Last Name</div><div class="alignCenter"><input type="text" name="lastName" placeholder="Do"><div class="error"></div></div>'+
							'<div class="padding">Email</div><div class="alignCenter"><input type="text" name="email" placeholder="johndo@email.com"><div class="error"></div></div>'+
							'<div class="padding">Location</div><div class="alignCenter"><input type="text" name="location" placeholder="123 Beach Ave, Santa Cruz"><div class="error"></div></div>'+
							'<input name="schedule" hidden>'+
							'<input name="admin" value="'+$.cookie('email')+'"hidden>'+
						'</form>'+
						createPaintTableHTML()+
						'<input id="confirm" type="button" class="alignRight button margin" value="Confirm"/>'+
					'</div>'+
				'</div>'
			);
			initPaintTable();
			$('#confirm').click(function () {
				if (!$.cookie("email")) removeCookies('/expired?msg=expired');
				renewLogin();
				var submit = true,
					firstName = $('#addNewUserForm input[name="firstName"]'),
					lastName = $('#addNewUserForm input[name="lastName"]'),
					email = $('#addNewUserForm input[name="email"]'),
					location = $('#addNewUserForm input[name="location"]');
				if (firstName.val() == '') { firstName.next().text('enter your first name'); submit = false; }
				else firstName.next().text('good');
				if (lastName.val() == '') { lastName.next().text('enter your last name'); submit = false; }
				else lastName.next().text('good');
				if (!checkEmail(email.val())) { email.next().text('email is invalid'); submit = false; }
				else email.next().text('good');
				if (location.val() == '') { location.next().text('enter a location'); submit = false; }
				else location.next().text('good');
				showError();
				if (submit) {
					$('#addNewUserForm input[name="schedule"]').val(parseSchedule());
					$('#addNewUserForm').submit();
				}
			});
		},
		promptWifi = function (input) {
			var currentSSID = input ? input : $.cookie('ssid');
			$('.pageContent').html(
				'<div class="margin">'+
					'<div class="message" hidden></div>'+
					'<h1>SDL will now connect to: '+currentSSID+'</h1>'+
					'<div class="padding">'+
						'<form class="center" id="setWifiForm" action="/setWifi" method="post">'+
							'<div class="padding">Network Name (SSID)</div><div class="alignCenter"><input name="ssid" type="text"><div class="error"></div></div>'+
							'<div class="padding">Password</div><div class="alignCenter"><input name="networkPW" type="password"><div class="error"></div></div>'+
							'<span class="indent"><input type="checkbox" name="noPW" value="noPW"><i class="padding">no password</i></span>'+
							'<input name="email" value="'+$.cookie('email')+'" hidden>'+
						'</form>'+
						'<input id="confirm" type="button" class="alignRight button margin" value="Confirm"/>'+
					'</div>'+
				'</div>'
			);
			$('#setWifiForm input[name="noPW"]').change(function () {
				var networkPW = $('#setWifiForm input[name="networkPW"]');
				if ($(this).is(":checked")) networkPW.attr('disabled', 'disabled').val('');
				else networkPW.attr('disabled', false);
			});
			$('#confirm').click(function () {
				if (!$.cookie("email")) removeCookies('/expired?msg=expired');
				renewLogin();
				var submit = true,
					ssid = $('#setWifiForm input[name="ssid"]'),
					networkPW = $('#setWifiForm input[name="networkPW"]'),
					noPW = $('#setWifiForm input[name="noPW"]');
				if (ssid.val() == '') { ssid.next().text('please enter a network name'); submit = false; }
				else ssid.next().text('good');
				if (networkPW.val() == '' && !noPW.is(":checked")) { networkPW.next().text('please enter the password for this network'); submit = false; }
				else networkPW.next().text('good');
				showError();
				if (submit) {
					$.cookie('ssid', $('input[name="ssid"]').val());
					$('#setWifiForm').submit();
				}
			});
		},
		resetPW = function () {
			$('.pageContent').html(
				'<div class="margin">'+
					'<div class="message" hidden></div>'+
					'<h1>Reset Password</h1>'+
					'<div class="padding">'+
						'<form class="center" id="setPasswordForm" action="/setPassword" method="post">'+
							'<div class="padding">Old Password</div><div class="alignCenter"><input name="oldPW" type="password"/><div class="error"></div></div>'+
							'<div class="padding">New Password</div><div class="alignCenter"><input name="newPW" type="password"/><div class="error"></div></div>' +
							'<div class="padding">Confirm Password</div><div class="alignCenter"><input name="newPWConfirm" type="password"/><div class="error"></div></div>' +
							'<input name="email" value="'+$.cookie('email')+'" hidden>'+
							'<input id="confirm" type="button" class="alignRight button margin" value="Confirm"/>'+
						'</form>'+
					'</div>'+
				'</div>'
			);
			$('#confirm').click(function () {
				if (!$.cookie("email")) removeCookies('/expired?msg=expired');
				renewLogin();
				var submit = true,
					oldPW = $('#setPasswordForm input[name="oldPW"]'),
					newPW = $('#setPasswordForm input[name="newPW"]'),
					newPWConfirm = $('#setPasswordForm input[name="newPWConfirm"]');
				if (!checkPassword(oldPW.val())) { oldPW.next().text('password is incorrect'); submit = false; }
				else oldPW.next().text('good');
				if (!checkPassword(newPW.val())) { newPW.next().text('at least 8 characters, 1 number, 1 lowercase, 1 upercase letter'); submit = false; }
				else newPW.next().text('good');
				if (!checkPassword(newPWConfirm.val())) { newPWConfirm.next().text('does not match'); submit = false; }
				else newPWConfirm.next().text('good');
				showError();
				if (submit) $('#setPasswordForm').submit();
			});
		},
		deleteAccount = function () {
			$('.pageContent').html(
				'<div class="margin">'+
					'<h1>Warning: This will remove your account and all associated users</h1>'+
					'<div class="padding">'+
						'<form class="center" id="deleteAccountForm" action="/deleteAccount" method="post">'+
							'<div class="padding">Password</div><div class="alignCenter"><input name="password" type="password"/><div class="error"></div></div>' +
							'<input name="email" value="'+$.cookie('email')+'" hidden>'+
							'<input id="confirm" type="button" class="alignRight button margin warning" value="Delete Account"/>'+
						'</form>'+
					'</div>'+
				'</div>'
			);
			$('#confirm').click(function () {
				if (!$.cookie("email")) removeCookies('/expired?msg=expired');
				var submit = true,
					password = $('#deleteAccountForm input[name="password"]');
				if (!checkPassword(password.val())) { password.next().text('invalid password'); submit = false; }
				else password.next().text('good');
				showError();
				if (submit) $('#deleteAccountForm').submit();
			});
		},
		notify = function () {
			if (!$.cookie('added')) $('.pageContent').html('<div class="margin"><h1>No new enrollments!</h1></div>');
			else {
				var added = $.cookie('added').split(',');
				$('.pageContent').html(
					'<div class="margin">'+
						'<div class="message" hidden></div>'+
						'<h1>Adding Finger ID <span id="newID">'+added[0]+'</span> to Database:</h1>'+
						'<div class="padding">'+
							'<form class="center" id="notifyForm" action="/clearNotification" method="post">'+
								'<div class="padding">First Name</div><div class="alignCenter"><input type="text" name="firstName" placeholder="John"><div class="error"></div></div>'+
								'<div class="padding">Last Name</div><div class="alignCenter"><input type="text" name="lastName" placeholder="Do"><div class="error"></div></div>'+
								'<div class="padding">Email</div><div class="alignCenter"><input type="text" name="email" placeholder="johndo@email.com"><div class="error"></div></div>'+
								'<div class="padding">Location</div><div class="alignCenter"><input type="text" name="location" placeholder="123 Beach Ave, Santa Cruz"><div class="error"></div></div>'+
								'<input name="id" hidden>'+
								'<input name="admin" value="'+$.cookie('email')+'" hidden>'+
								'<input name="toAdd" value="'+$.cookie('added')+'" hidden>'+
							'</form>'+
							'<input id="confirm" type="button" class="alignRight button margin" value="Confirm"/>'+
						'</div>'+
					'</div>'
				);
				$('#confirm').click(function () {
					if (!$.cookie("email")) removeCookies('/expired?msg=expired');
					renewLogin();
					var submit = true,
						firstName = $('#notifyForm input[name="firstName"]'),
						lastName = $('#notifyForm input[name="lastName"]'),
						email = $('#notifyForm input[name="email"]'),
						id = $('#notifyForm input[name="id"]'),
						location = $('#notifyForm input[name="location"]'),
						toAdd = $('#notifyForm input[name="toAdd"]');
					if (firstName.val() == '') { firstName.next().text('enter your first name'); submit = false; }
					else firstName.next().text('good');
					if (lastName.val() == '') { lastName.next().text('enter your last name'); submit = false; }
					else lastName.next().text('good');
					if (!checkEmail(email.val())) { email.next().text('email is invalid'); submit = false; }
					else email.next().text('good');
					if (location.val() == '') { location.next().text('enter a location'); submit = false; }
					else location.next().text('good');
					showError();
					if (submit) {
						added = $.cookie('added').split(',');
						id.val(added[0]);
						added.shift();
						toAdd.val(added.join(','));
						$.cookie('added', toAdd.val());
						if (added.length == 0) $('.pageContent').html('<div class="margin"><h1>No new enrollments!</h1></div>');
						$('#notifyForm').submit();
					}
				});
			}
		},
		createPaintTableHTML = function () {
			var htmlString = '<div class="padding">'+
								'<div class="alignLeft input">'+
									'<div class="label">Use template as default</div>'+
									'<div><select name="scheduleTemplate">'+
										'<option value="'+FULL_ACCESS+'" selected>Full Access</option>';
			var templates = $($('.hiddenInfo')[0]).text().split(';');
				for (var i = 0; i < templates.length; i++) {
					var template = templates[i].split(',');
					htmlString += '<option value="'+template[1]+'" selected>'+template[0]+'</option>';
				}	
				htmlString +=
									'</select></div>'+
									'<div id="template" class="hoverbox" hidden>'+
										'<input type="text" name="name" class="stretch" placeholder="name of schedule"/>'+
										'<input type="button" class="stretch button" value="Save as template"/>'+
									'</div>'+
								'</div>'+
								'<div class="alignLeft input">'+
									'<div class="label">Edit Week</div>'+
									'<div><input type="week" name="weekYear" value="'+d.getFullYear()+'-W'+d.getWeek()+'"></div>'+
								'</div>'+
								'<div class="selection button alignRight glyphicon glyphicon-erase" value="erase"></div>'+
								'<div class="selection button alignRight selected glyphicon glyphicon-pencil" value="paint"></div>'+
							'</div>'+
							'<div class="padding">'+
								'<div class="stretch scrollPadding">'+
									'<table class="stretch alignCenter">'+
										'<tr class="alignCenter bold"><td class="paintHeader">TIME</td><td class="paintHeader">SUN</td><td class="paintHeader">MON</td><td class="paintHeader">TUE</td><td class="paintHeader">WED</td><td class="paintHeader">THU</td><td class="paintHeader">FRI</td><td class="paintHeader">SAT</td></tr>'+
									'</table>'+
								'</div>'+
								'<div class="stretch paintContainer">'+
									'<table class="stretch alignCenter paintTable">';
			for (var i = 0; i < 48; i++) {
				htmlString += '<tr class="paintSlot">';
				htmlString += '<td class="timeSlot">'+Math.floor(i/2)+':'+(i%2==1?'30':'00')+'</td>';
				for (var j = 0; j < 7; j++)
					htmlString += '<td class="paintDay"></td>'; 
				htmlString += '</tr>';
			}		 
			htmlString += '</table></div></div>';
			return htmlString;
		},
		initPaintTable = function (schedule) {
			$('.paintDay').mouseover(function () {
				if (mouseState == 'down') {
					if ($('.selection.selected').attr('value') == 'paint') $(this).addClass('painted');
					else if ($('.selection.selected').attr('value')  == 'erase') $(this).removeClass('painted');
				}
			});
			$('.paintDay').mousedown(function () {
				if ($('.selection.selected').attr('value')  == 'paint') $(this).addClass('painted');
				else if ($('.selection.selected').attr('value') == 'erase') $(this).removeClass('painted');
			});
			$('.selection').click(function () {
				if (!$.cookie("email")) removeCookies('/expired?msg=expired');
				renewLogin();
				$('.selection').removeClass('selected');
				$(this).addClass('selected');
			});
			if (!schedule) return;
			$('.paintDay').removeClass('painted');
			for (var i = 0; i < 48; i++) {
				var mask = 0b01000000;
				for (var j = 1; j < 8; j++) {
					if ((schedule.charCodeAt(i) & mask) != 0) {
						$($($('.paintSlot')[i]).children()[j]).addClass('painted');
					}
					mask >>= 1;
				}
			}
		},
		parseSchedule = function () {
			var byteArray = [],
				outputString = "";
			$('.paintSlot').each(function (time) {
				byteArray[time] = 0x00;
				$(this).children().each(function (day) {
					if ($(this).hasClass('painted'))
						byteArray[time] |= (0b10000000 >> day);
				});
				outputString += String.fromCharCode(byteArray[time]);
			});
			return outputString;
		},
		getWeekFromSchedule = function (string, year, week) {
			var schedule = string.split(';');
			if (schedule) {
				for (var i in schedule) {
					var target = schedule[i].split(',');
					if (target[0] == year && target[1] == week)
						return target[2];
				}
			}
			return FULL_ACCESS;
		},
		displaySchedule = function () {
			$('.edit td:nth-child(6)').each(function() {
				var htmlString = "";
				for (var day in parseDays($(this).text())) htmlString += '<span class="openDay">'+daysOfWeek[day]+'</span>';
				$(this).next().html(htmlString);
			});
		},
		parseDays = function (scheduleString) {
			var daysAllowed = [];
			for (var i = 0; i < 11; i++) {
				var mask = 0b01000000;
				for (var j = 0; j < 7; j++) {
					if (daysAllowed[j]) { mask >>= 1; continue; }
					if ((scheduleString.charCodeAt(i) & mask) != 0) daysAllowed[j] = true;
					mask >>= 1;
				}
			}
			return daysAllowed;
		},
		checkEmail = function (email) {
			var pattern = /^(([^<>()[\]\\.,;:\s@\"]+(\.[^<>()[\]\\.,;:\s@\"]+)*)|(\".+\"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;
			if(pattern.test(email)) return true;
			else return false;
		},
		checkPassword = function (password) {
			var pattern = /^.*(?=.{8,})(?=.*\d)(?=.*[a-z])(?=.*[A-Z]).*$/;
			if (pattern.test(password)) return true;
			else return false;
		},
		checkProductID = function (id) {
			var pattern = id.split('.');
			if (pattern.length != 4) return false;
			for (var i in pattern) {
				if (pattern[i] < 0 || pattern[i] > 255) return false;
			}
			return true;
		},
		showError = function () {
			$('.error').each(function () {
				if ($(this).text() == '' || $(this).text() == 'good') $(this).css('background', 'transparent');
				else $(this).css('background', 'rgba(255,0,0,0.7)');
			});
		},
		removeCookies = function (link) {
			var cookies = $.cookie();
			for (var cookie in cookies) $.removeCookie(cookie);
			window.location.replace(link);
		},
		renewLogin = function () {
			var date = new Date();
			var minutes = 30;
			date.setTime(date.getTime() + (minutes * 60 * 1000));
			$.cookie("email", $.cookie("email"), { expires: date });
		},
		initEdit = function () {
			$('.edit').click(function () {
				if (!$.cookie("email")) removeCookies('/expired?msg=expired');
				$('.binded').removeClass('binded');
				$(this).addClass('binded');
				renewLogin();
				var s = $(this).children(),
					id = $.trim($(s[1]).text()),
					email = $.trim($(s[2]).text()),
					firstName = $.trim($(s[3]).addClass('binded').text()),
					lastName = $.trim($(s[4]).addClass('binded').text()),
					scheduleString = $.trim($(s[5]).text());
					$(s[6]).addClass('binded');
				$('.pageContent').html(
					'<div class="margin">'+
						'<div class="userNavMenu">'+
							'<span class="button alignRight" id="renameTab">Rename</span>'+
							'<span class="button alignRight selected" id="scheduleTab">Schedule</span>'+
						'</div>'+
						'<div class="message" hidden></div>'+
						'<h1 id="username">' + firstName + ' ' + lastName + '</h1>'+
						'<form id="deleteUser" action="/delete" method="post" hidden>'+
							'<input type="password" name="email" value="'+ email +'"/>'+
							'<input type="password" name="admin" value="'+ $.cookie('email') +'"/>'+
						'</form>' +
						'<form id="scheduleUser" action="/schedule" method="post">'+
							'<input type="password" name="email" value="'+ email +'" hidden/>'+
							'<input type="password" name="admin" value="'+ $.cookie('email') +'" hidden/>'+
							'<input type="password" name="schedule" hidden/>'+
							createPaintTableHTML() +
						'</form>' +
						'<form id="renameUser" action="/rename" method="post" hidden>'+
							'<input type="password" name="email" value="'+ email +'" hidden>'+
							'<input type="password" name="admin" value="'+ $.cookie('email') +'" hidden>'+
							'<div class="padding">First Name</div><div class="alignCenter"><input name="firstName" type="text" value="'+ firstName +'"/><div class="error"></div></div>'+
							'<div class="padding">Last Name</div><div class="alignCenter"><input name="lastName" type="text" value="'+ lastName +'"/><div class="error"></div></div>' +
						'</form>'+
						'<form id="saveTemplate" action="/saveTemplate" method="post" hidden>'+
							'<input type="password" name="name"/>'+
							'<input type="password" name="schedule"/>'+
							'<input type="password" name="email" value="'+ email +'"/>'+
							'<input type="password" name="admin" value="'+ $.cookie('email') +'"/>'+
						'</form>' +
						'<input id="confirm" type="button" class="alignRight button" value="Save"/>'+
						'<input id="delete"  type="button" class="alignLeft button" value="Delete user"/>'+
					'</div>'
				);
				initPaintTable(getWeekFromSchedule(scheduleString), d.getFullYear(), d.getWeek());
				$('#renameTab').click(function(){
					$('.button').removeClass('selected');
					$(this).addClass('selected');
					$('form').hide();
					$('#renameUser').show();
				});
				$('#scheduleTab').click(function(){
					$('.button').removeClass('selected');
					$(this).addClass('selected');
					$('form').hide();
					$('#scheduleUser').show();
				});
				$("#confirm").click(function () {
					if (!$.cookie("email")) removeCookies('/expired?msg=expired');
					renewLogin();
					if ($('#scheduleTab').hasClass('selected')) {
						$('#scheduleUser input[name="schedule"]').val(parseSchedule());
						$('#scheduleUser').submit();
					} else $('#renameUser').submit();
				});
				$('#delete').click(function () {
					if (!$.cookie("email")) removeCookies('/expired?msg=expired');
					renewLogin();
					$('#deleteUser').submit();
				});
				$('#scheduleUser input[name="weekYear"]').change(function(){
					var scheduleString = $.trim($($('.edit.binded').children()[5]).text()),
						weekYear = $(this).val().split('-W');
					initPaintTable(getWeekFromSchedule(scheduleString, weekYear[0], weekYear[1]));
				});
				$('#scheduleUser select[name="scheduleTemplate"]').change(function(){
					initPaintTable($(this).val());
				}).mouseenter(function(){ $('#template').show(); }).click(function() {$('#template').hide()});
				$('#template').focusout(function(){ $(this).hide(); });
				$('#template input[name="name"]').css('width','100%');
				$('#template input[type="button"]').click(function(e){
					$('#saveTemplate input[name="name"]').val($('#template input[name="name"]').val());
					$('#saveTemplate input[name="schedule"]').val(parseSchedule());
					$('#saveTemplate').submit();
					e.stopPropagation();
				});
				formSubmission();
			});
		},
		formSubmission = function () {
			$('form').submit(function(e) {
				var form = $(this);
				$.ajax({
					url: $(this).attr('action'), type: $(this).attr('method'), data: $(this).serialize(), success: function(string) {
						var obj;
						try {
							obj = JSON.parse(string);
							if (typeof obj == 'object') {
								$('.message').show()
									.html('<span class="openDay">'+obj.label+'</span><div class="margin">'+obj.msg+'</div>')
									.animate({opacity: 1}, 250);
								$('.message').click(function () {
									$(this).animate({opacity: 0}, 250, function () {$(this).hide();})
								});
								$(".mainPage").animate({ scrollTop: "0px" });
								switch (obj.action) {
									case 'setWifi': break;
									case 'setPassword': break;
									case 'deleteAccount':
										if (obj.label == 'incorrectPW') $('#deleteAccountForm input[name="password"]').next().text('password is incorrect');
										else if (obj.label == 'success') removeCookies('/expired?msg=accountDelete');
										break;
									case 'rename':
										if (obj.label == 'Fail') break;
										$('#username').text(obj.firstName + ' ' + obj.lastName);
										$('.binded').each(function() {
											if ($(this).hasClass('firstnameColumn')) $(this).text(obj.firstName);
											else if ($(this).hasClass('lastnameColumn')) $(this).text(obj.lastName);
										});
										break;
									case 'schedule':
										if (obj.label == 'Fail') break;
										$('.binded').each(function() {
											if ($(this).hasClass('scheduleColumn')) {
												var htmlString = "";
												for (var day in parseDays(obj.schedule)) htmlString += '<span class="openDay">'+daysOfWeek[day]+'</span>';
												$(this).html(htmlString);
											}
										});
										break;
									case 'delete':
										if (obj.label == 'Fail') break;
										$('.binded').each(function() {
											if ($(this).hasClass('edit')) {
												$(this).animate({opacity: 0}, 500, function () {$(this).remove();});
												$('.message').siblings().each(function() {
													$(this).animate({opacity: 0}, 500, function () {$(this).remove();});
												});
											}
										});
										break;
									case 'enroll':
										if (obj.label == 'Fail') break;
										$('.edit td:nth-child(3)').each(function() {
											if ($(this).text() == obj.admin) {
												$($(this).parent()).after(obj.html);
												refreshData();
											}
										});
										break;
									case 'notify':
										if (obj.label == 'Fail') break;
										var added = $('#notifyForm input[name="toAdd"]').split(',');
										if (!added[0]) {
											 $('.pageContent').html('<div class="margin"><h1>No new enrollments!</h1></div>');
										} else {
											$('#newID').text(added[0]);
											$('#notifyForm input[name="id"]').val(added[0]);
										}
										break;
									case 'saveTemplate':
										if (obj.label == 'Fail') break;
										$('#scheduleUser select[name="scheduleTemplate"]').append('<option value="'+obj.value+'">'+obj.name+'</option>');
										break;
								}
								showError();
							}
						} catch (err) {
							form.unbind('submit').submit();
						}
					}
				});
				e.preventDefault();
			});
		},
		refreshData = function () {
			initEdit();
			displaySchedule();
		};
	
	/************************************************************************************
		Event State Machine
	*************************************************************************************/
	var mouseState = 'up';
	$(window).mousedown(function() { mouseState = 'down'; });
	$(window).mouseup(function() { mouseState = 'up'; });
	$('.item').click(function () {
		$('.item').removeClass('selected');
		$(this).addClass('selected');
		switch (this.id) {
			case 'setWifi': promptWifi(); break;
			case 'deleteAccount': deleteAccount(); break;
			case 'resetPW': resetPW(); break;
			case 'addNewUser': enrollUser(); break;
			case 'logout': removeCookies('/'); break;
			case 'notification': notify(); break;
		}
		formSubmission();
	});
	$('.hideShow').click(function (e) {
		var admin = $($(this).siblings()[6]).text();
		if ($(this).hasClass('hideUsers')) {
			$(this).html('&#9660');
			$('.edit').each(function () {
				var list = $(this).children();
				if ($(list[2]).text() != admin && $(list[7]).text() == admin) $(this).animate({opacity: 0}, 200, function() {$(this).hide();});
			});
		} else {
			$(this).html('&#9650');
			$('.edit').each(function () {
				var list = $(this).children();
				if ($(list[2]).text() != admin && $(list[7]).text() == admin) $(this).show().animate({opacity: 1}, 200);
			});
		}
		$(this).toggleClass('hideUsers');
		e.stopPropagation();
	});
	setInterval(function () {
		$.ajax({
			url: 'update', type: 'POST', data: {email: $.cookie('email')}, success: function(string) {
				if (string == 'None') {
					$('#newMsg').text('0');
					return;
				}
				var qStr = string.split('&'),
					added = qStr[0] ? qStr[0].split(',') : [], uniqueAdded = [],
					deleted = qStr[1] ? qStr[1].split(',') : [];
					$.each(added, function(i, el){ if($.inArray(el, uniqueAdded) === -1 && el != "") uniqueAdded.push(el); });
				$('#newID').text(uniqueAdded[0]);
				for (var i in deleted) {
					$('.idColumn').each(function() {
						if ($(this).text() == deleted[i] && $($(this).parent().children()[7]).text() == $.cookie('email')) {
							var edit = $(this).parent();
							edit.animate({opacity: 0}, 500, function () {$(this).remove();});
							$('.message').siblings().each(function() {
								edit.animate({opacity: 0}, 500, function () {$(this).remove();});
							});
						}
					});
				}
				$('#newMsg').text(uniqueAdded.length);
				$.cookie('added', uniqueAdded.join(','));
			}
		});
	}, 5000);
	refreshData();
});

/************************************************************************************
	Extension
*************************************************************************************/

Date.prototype.getWeek = function() {
	var onejan = new Date(this.getFullYear(),0,1);
	return Math.ceil((((this - onejan) / 86400000) + onejan.getDay()+1)/7);
}
