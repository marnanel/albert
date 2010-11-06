use strict;
use warnings;

open PROCESSOR, "<src/Processor.cpp" or die;

my %opcodes;

my $mnemonic = 'XXX';
my %mnemonics;

while (<PROCESSOR>) {
    if (m!case OP_(...):(.*)!) {
	$mnemonic = $1;
	my $comment = $2;

	if ($mnemonic ne 'XXX') {
	    $comment = ' // ' unless $comment;
	    $mnemonics{$1} = $comment;
	}
    }

    if (m!OpcodeDetails\(0x(..), (MODE_[^,]*),\s*(\d+)!) {
	$opcodes{hex($1)} = sprintf("{ OP_%s, %-20s %d }",
				    $mnemonic,
				    $2.',',
				    $3);
    }
}

close PROCESSOR or die;

open OPCODES, ">src/opnames.cpp" or die;

print OPCODES "// Generated file.  Do not edit.  Do not check in.\n";

################################################################

print OPCODES "\n";
print OPCODES "typedef enum _Op {\n";
for my $mnemonic (sort keys %mnemonics) {
    print OPCODES "  OP_$mnemonic, $mnemonics{$mnemonic}\n";
}
print OPCODES "\n";
print OPCODES "  OP_XXX // Fallback\n";
print OPCODES "} Op;\n";

################################################################

print OPCODES "\n";
print OPCODES "QString opnames =\n";
for my $mnemonic (sort keys %mnemonics) {
    print OPCODES "  \"$mnemonic \"\n";
}
print OPCODES "  \"XXX \";\n";

################################################################

close OPCODES or die;

open OPCODES, ">src/opcodes.cpp" or die;

print OPCODES "// Generated file.  Do not edit.  Do not check in.\n";

################################################################

print OPCODES "struct Opcode opcodes[256] = {\n";
for (my $i=0; $i<256; $i++) {
    my $str = '{ OP_XXX, MODE_IMPLIED,        0 }';

    $str = $opcodes{$i} if defined $opcodes{$i};

    printf OPCODES (sprintf "  %s, // %02x\n",
		    $str, $i);
}

print OPCODES "};\n";

close OPCODES or die;
