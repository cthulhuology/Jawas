#!/bin/env perl

my (@lines) = split "\n", `cat *.c`;

my @excluded  = qw/ 
	char int void str void static size_t HEADER_FUNC Socket reg TLSInfo TLSSocket  struct uint64_t
	Headers Usage Response Request RequestInfo Server NOT_IMPLEMENTED extern MethodDispatch
	lua_State jmp_buf const typedef 
	Client Region Database MimeTypes IPAddress File
/;

my @string_excludes = qw/ 
	_ at len blank set  clone copy cmp from ref search find append
	append_header Key Value new_header find_header
	reserve advance
	str2lua lua2str
/;


push @excluded, @string_excludes unless ($ARGV[0] eq '-s');

my @functions = ();

sub not_function {
	$_[0] && (
	$_[0] =~ /^\W+/ ||
	$_[0] =~ /^\s+/ ||
	scalar(grep { $_[0] eq $_ } @excluded)); 
}

for (@lines) {
	if (/^([a-zA-Z0-9_]+)/) {
		next if not_function($1);
		push @functions, $1;
	}
}

my @stack = ();

our %datum;
our %styles;

our %subgraphs;

my (@files) = split "\n", `ls -1 *.c`;
for $f (@files) {
	$subgraphs{$f} = [];
	for $l (split "\n", `cat $f`) {
		if ($l =~ /^([a-zA-Z0-9_]+)/) {
			if (! not_function($1)) {
				push @stack, $1;
				push @{$subgraphs{$f}}, $1;
			}
			next;
		}
		if (@stack && $l =~ /^}/) {
			pop @stack;
			next;
		}
		if (@stack) {
			for $f (grep { $l =~ /\W+$_\(/ } @functions) {
				next if (not_function($f));
				next if ($f eq $stack[-1]);
				next if ($f eq "_" or $stack[-1] eq "_");
				$datum{ "$stack[-1] -> $f"} += 1;
				$styles{ $stack[-1] } += 1;
			}
		}
	}
}

for (grep { !not_function($_) } @functions) { 
	$styles{$_} = /Lua/ ? "color=pink,shape=polygon,style=filled":
		($styles{$_} > 0) ?
			"color=lightblue,style=filled":
			"color=green,shape=box,style=filled";
}

print "digraph G {\n";
print "$_ [weight=$datum{$_}];\n" for (sort keys %datum);
print "$_ [$styles{$_}];\n" for (sort keys %styles);
print "subgraph \"$_\" { " . join("; ", @{$subgraphs{$_}}) . " }\n" for (sort keys %subgraphs);
print "}\n";
