$( document ).ready(function() {
	
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
		Input Checking
	*************************************************************************************/
	var checkEmail = function (email) {
			var pattern = /^(([^<>()[\]\\.,;:\s@\"]+(\.[^<>()[\]\\.,;:\s@\"]+)*)|(\".+\"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;
			if(pattern.test(email)) return true;
			else return false;
		},
		checkPassword = function (password) {
			var pattern = /^.*(?=.{8,})(?=.*\d)(?=.*[a-z])(?=.*[A-Z]).*$/;
			if (pattern.test(password)) return true;
			else return false;
		},
		checkproductID = function (id) {
			var pattern = id.split('.');
			if (pattern.length != 4) return false;
			for (var i in pattern) {
				if (pattern[i] < 0 || pattern[i] > 255) return false;
			}
			return true;
		};
		
	// Default to selecting login tab
	$('form').hide();
	$('#loginForm').show();
	
	// React to clicks on tabs
	$('.item').click(function () {
		$('.item').removeClass('selected');
		$(this).addClass('selected');
		$('form').hide();
		$('#'+this.id+'Form').show();
	});
	
	/************************************************************************************
		Submission Response Handler
	*************************************************************************************/
	$('form').submit(function(e) {
		var form = $(this);
		$.ajax({
			url: $(this).attr('action'), type: $(this).attr('method'), data: $(this).serialize(), success: function(string) {
				var obj;
				try {
					obj = JSON.parse(string);
					if (typeof obj == 'object') {
						switch (obj.action) {
							case 'register':
								var email = $('#registerForm input[name="email"]'),
									password = $('#registerForm input[name="password"]');
								if (obj.label == "email") {
									email.next().text(obj.msg);
								} else if (obj.label == "success") {
									$('#loginForm input[name="email"]').val(email.val());
									$('#loginForm input[name="password"]').val(password.val());
									setTimeout(function () { $('#loginForm').submit(); }, 500);
								}
								break;
							case 'account':
								var email = $('#loginForm input[name="email"]'),
									password = $('#loginForm input[name="password"]');
								if (obj.label == "userNotFound") {
									email.next().text(obj.msg);
								} else if (obj.label == "incorrectPW") {
									password.next().text(obj.msg);
								}
								break;
							case 'forgotPassword':
								var email = $('#forgotPasswordForm input[name="email"]');
								email.next().text(obj.msg);
								break;
						}
					}
					$('.error').each(function () {
						if ($(this).text() == '' || $(this).text() == 'good') $(this).css('background', 'transparent');
						else $(this).css('background', 'rgba(255,0,0,0.7)');
					});
				} catch (err) {
					form.unbind('submit').submit();
				}
			}
		});
		e.preventDefault();
	});
	$('.item#login').addClass('selected');
	document.onkeydown = function(){
		if (window.event.keyCode == 13)	$('#'+$('.selected').attr('id')+'.button').trigger('click');
	};
	$('.button').click(function (e) {
		var submit = true, firstName, lastName, email, password, passwordConfirm, productID, ssid, networkPW;
		if (this.id == 'login') {
			email = $('#loginForm input[name="email"]');
			password = $('#loginForm input[name="password"]');
			if (!checkEmail(email.val()) || email == '') { email.next().text('email is invalid'); submit = false; }
			else email.next().text('good');
			if (!checkPassword(password.val()) || password == '') { password.next().text('at least 8 characters, 1 number, 1 lowercase, 1 upercase letter'); submit = false; }
			else password.next().text('good');
		} else if (this.id == 'register') {
			firstName = $('#registerForm input[name="firstName"]');
			lastName = $('#registerForm input[name="lastName"]');
			email = $('#registerForm input[name="email"]');
			password = $('#registerForm input[name="password"]');
			passwordConfirm = $('#registerForm input[name="passwordConfirm"]');
			productID = $('#registerForm input[name="productID"]');
			if (firstName.val() == '') { firstName.next().text('enter your first name'); submit = false; }
			else firstName.next().text('good');
			if (lastName.val() == '') { lastName.next().text('enter your last name'); submit = false; }
			else lastName.next().text('good');
			if (!checkEmail(email.val())) { email.next().text('email is invalid'); submit = false; }
			else email.next().text('good');
			if (!checkPassword(password.val())) { password.next().text('at least 8 characters, 1 number, 1 lowercase, 1 upercase letter'); submit = false; }
			else password.next().text('good');
			if (password.val() != passwordConfirm.val() || password.val() == '') { passwordConfirm.next().text('password does not match!'); submit = false; }
			else passwordConfirm.next().text('good');
			if (!checkproductID(productID.val()) || productID == '') { productID.next().text('invalid product ID'); submit = false; }
			else productID.next().text('good');
		} else if (this.id == 'forgotPassword') {
			email = $('#forgotPasswordForm input[name="email"]');
			if (!checkEmail(email.val()) || email == '') { email.next().text('email is invalid'); submit = false; }
			else email.next().text('good');
		}
		$('.error').each(function () {
			if ($(this).text() == '' || $(this).text() == 'good') $(this).css('background', 'transparent');
			else $(this).css('background', 'rgba(255,0,0,0.7)');
		});
		if (submit) {
			var date = new Date();
			var minutes = 15;
			date.setTime(date.getTime() + (minutes * 60 * 1000));
			$.cookie("email", email.val(), { expires: date });
			$('#'+this.id+'Form').submit();
		}
	});
});
